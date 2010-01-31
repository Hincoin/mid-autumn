#ifndef MA_INCLUDE_MEMORY_POOL_HPP
#define MA_INCLUDE_MEMORY_POOL_HPP

//#include <boost/intrusive/set.hpp>
#include <functional>
#include <limits>
#include <cstring>
#include <cstdlib>
#include <new>
#ifdef _DEBUG
#include <algorithm>
#endif // _DEBUG
#include <cassert>
#ifdef _GNU_C_
#include <stdint.h>
#endif

#include "details/intrusive_rbtree.hpp"
#include "details/intrusive_list.hpp"
#include "details/pool_mutex.hpp"

#include "details/common_functions.hpp"

//todo: 
//mutex multi-thread support
//more-secure small allocation condition

#ifdef _MSC_VER
	//msvc compiler
#pragma warning(push)
#pragma warning( disable : 4267)
#endif

namespace ma{
	namespace core{

		//////////////////////////////////////////////////////////////////////////

		struct MemBlock;
		struct comp_mem_block:std::binary_function<MemBlock*,MemBlock*,bool>
		{
			bool operator()(const MemBlock* lhs,const MemBlock* rhs)const;
		};

		struct FreeBlock: public intrusive_multi_rbtree<FreeBlock>::node
			//boost::intrusive::set_base_hook<boost::intrusive::optimize_size<false>
			//,boost::intrusive::link_mode<boost::intrusive::normal_link> >
		{
			const MemBlock* getMemBlock()const;
			MemBlock* getMemBlock();
			friend bool operator<(const FreeBlock& a,const FreeBlock& b){
				return comp_mem_block()(a.getMemBlock(),b.getMemBlock());
			}
			friend bool operator>(const FreeBlock& lhs,const FreeBlock& rhs){
				return comp_mem_block()(rhs.getMemBlock(),lhs.getMemBlock());
			}
			bool operator<(size_t sz)const;
			bool operator>(size_t sz)const;
			FreeBlock(){}
		private:
			FreeBlock(const FreeBlock&);
			FreeBlock& operator=(const FreeBlock&);
		};
		// small free nodes is an optimization for the large allocator
		// these blocks are never queried so they are simply kept in a linked list
		// this way inserts and erases are much faster
		struct small_free_node : public intrusive_list<small_free_node>::node {
			const MemBlock*  getMemBlock()const;
			MemBlock*  getMemBlock();
		};
		typedef intrusive_list<small_free_node> small_free_node_list;

		struct MemBlock{
			MemBlock(MemBlock* prev,size_t sz):prev(prev),size(sz),used(false){assert(size > sizeof(FreeBlock));}
			MemBlock* next(){/*assert(used || size >= sizeof(FreeBlock));*/ return (MemBlock*)(((char*)(this+1))+size);}
			FreeBlock* free_block(){/*assert(!used && size >= sizeof(FreeBlock));*/return ((FreeBlock*)(this+1));}
			small_free_node* small_free_block(){/*assert(!used && size > sizeof(small_free_node));*/return ((small_free_node*)(this+1));}

			MemBlock* prev;
			size_t size:sizeof(std::size_t)*8-1;
			size_t used:1;
			typedef size_t size_type;//:sizeof(std::size_t)*8-1;
		};

		inline const MemBlock* small_free_node::getMemBlock()const
		{
			return (MemBlock*)((const char*)(this)-sizeof(MemBlock));
		}
		inline MemBlock* small_free_node::getMemBlock()
		{
			return (MemBlock*)((char*)(this)-sizeof(MemBlock));
		}
		inline const MemBlock* FreeBlock::getMemBlock()const
		{
			return (MemBlock*)((const char*)(this)-sizeof(MemBlock));
		}
		inline MemBlock* FreeBlock::getMemBlock()
		{
			return (MemBlock*)((char*)(this)-sizeof(MemBlock));
		}
		inline bool FreeBlock::operator<(size_t sz)const{
			return getMemBlock()->size < sz;
		}
		inline bool FreeBlock::operator>(size_t sz)const{
			return getMemBlock()->size > sz;
		}
		inline bool comp_mem_block::operator ()(const MemBlock* lhs,const MemBlock* rhs)const
		{
			return lhs->size < rhs->size /*|| 
				!(rhs->size < lhs->size) && lhs < rhs*/;
		}

		//memblock-freeblock-rawmemory
		template<typename Mutex>
		struct big_memory_pool{
			void* alloc(size_t sz);
			void* realloc(void* ptr,size_t size){
				void* newPtr = tree_realloc(ptr, size );
				return newPtr;
			}
			void free(void* ptr){
				tree_free(ptr);
			}
			bool release_memory(size_t mem_size = -1){
				return tree_recycle(mem_size);
			}
			//debug
			bool checkValid(MemBlock* block)
			{
				if (!enable_debug)
				{
					return true;
				}
#ifdef _DEBUG
				assert(block);
				if(block->prev)
					assert(reinterpret_cast<char*>(block) == (char*)(block->prev+1)+block->prev->size);
				return block->size != 0;
#else 
				return true;
#endif
			}
			void enableDebug(bool flag)
			{
				enable_debug = flag;
			}
			bool checkAllBlocks(){
				if (!enable_debug)
				{
					return true;
				}
				bool ret = true;
#ifdef _DEBUG	
				for(block_set_t::iterator it = block_set_.begin();it != block_set_.end(); ++it)
				{
					assert( (ret = checkValid(it->getMemBlock())) && !(it->getMemBlock())->used);
					MemBlock* prev = (*it).getMemBlock();
					while(prev && !prev->used)
					{
						assert(cached_block_ == prev || !check_not_free_block(prev));
						assert(prev->next()->used);
						prev = prev->prev;
					}
				}
				if(cached_block_)
				{
					for (block_set_t::iterator it = block_set_.lower_bound(*cached_block_->free_block());
						it != block_set_.upper_bound(*cached_block_->free_block());
						++it)
					{
						assert(cached_block_ != it->getMemBlock());
					}
					assert(checkValid(cached_block_));
				}
#endif
				return ret;
			}
			bool check_not_free_block(MemBlock* blk)
			{
				if (blk == mr_block_ || blk == cached_block_)
				{
					return false;
				}
				for (block_set_t::iterator it = block_set_.lower_bound(*blk->free_block());
					it != block_set_.upper_bound(*blk->free_block());
					++it)
				{
					if( blk == it->getMemBlock())return false;
				}
				for (small_free_node_list::iterator it = small_free_list.begin();
					it != small_free_list.end();++it)
				{
					if(blk == it->getMemBlock())return false;
				}
				return true;
			}
			size_t debug_total_size;
			size_t debug_allocated_size;
			size_t debug_find_count;
			size_t debug_found_count;
			size_t debug_insert_count;
			bool enable_debug;

			big_memory_pool():cached_block_(0),mr_block_(0){debug_total_size = 0;debug_find_count = 0;debug_found_count=0;debug_allocated_size = 0;debug_insert_count = 0;enable_debug = false;}
			~big_memory_pool(){
				//return all memory to OS
				release_memory(size_t(-1));
				assert (block_set_.empty());
				assert (small_free_list.empty());
			}

		private:
			big_memory_pool(const big_memory_pool&)
			{
				assert(false);
			}
			big_memory_pool& operator=(const big_memory_pool&)
			{
				assert(false);
			}
			//typedef std::set<MemBlock*,comp_mem_block> block_set_t;
			//typedef boost::intrusive::set<FreeBlock,boost::intrusive::compare<std::less<FreeBlock> > > 
			//	block_set_t;
			//typedef boost::intrusive::multiset<FreeBlock,boost::intrusive::compare<std::less<FreeBlock> > > 
			//	block_set_t;
			//typedef boost::intrusive::rbtree<FreeBlock,boost::intrusive::compare<std::less<FreeBlock> > >
			//	block_set_t;
			typedef intrusive_multi_rbtree<FreeBlock> block_set_t;
			block_set_t block_set_;
			MemBlock* cached_block_;//cached most big block
			MemBlock* mr_block_;//cached most recently used block
			small_free_node_list small_free_list;
			Mutex mutex_;
			// the maximum small allocation, anything larger goes to the tree allocator
			// must be a power of two
			static const size_t MAX_SMALL_ALLOCATION_LOG2 = 8UL;
			static const size_t MAX_SMALL_ALLOCATION  = 1UL << MAX_SMALL_ALLOCATION_LOG2;
			typedef details::scope_lock<Mutex> scope_lock_t;
		private:
			void* tree_alloc(size_t sz);
			void* tree_realloc(void* p,size_t sz);
			void tree_free(void* ptr);
			bool tree_recycle(size_t mem_size);
			//
			void insert_free_block(MemBlock* m)
			{
				assert((! m->prev || m->prev->used) && 
					m->next()->used);
				if(mr_block_)
				{
					if (mr_block_->size < MAX_SMALL_ALLOCATION)
						small_free_list.push_back(mr_block_->small_free_block());
					else
					{				
						block_set_.insert((mr_block_->free_block()) );
						debug_insert_count++;
					}
				}

				mr_block_ = m;
				//block_set_.insert(*m->free_block());
				//block_set_.insert_equal(*m->free_block());
			}
			void insert_free_block(block_set_t::iterator hint,MemBlock* m)
			{
				insert_free_block(m);
				//block_set_.insert(hint,*m->free_block());
				//block_set_.insert_equal(hint,*m->free_block());
			}
			void erase_free_block(MemBlock* m)
			{
				if (m == mr_block_)
					mr_block_ = 0;
				else if(m->size < MAX_SMALL_ALLOCATION)
					small_free_list.erase(m->small_free_block());
				else
					block_set_.erase(m->free_block());
			}
			//split the block and return next block
			MemBlock* split_block(MemBlock* m,size_t sz){
				if(m->size > sz+sizeof(MemBlock)+sizeof(FreeBlock))
				{
					MemBlock* next = (MemBlock*)((char*)(m+1) + sz);
					next->prev = m;
					next->size = m->size- MemBlock::size_type(sz + sizeof(MemBlock)); 
					MemBlock* m_next = m->next();//reinterpret_cast<MemBlock*>(reinterpret_cast<char*>(m+1) + m->size);
					m_next->prev = next;

					m->size = MemBlock::size_type(sz);
					assert(checkValid(m));
					assert(checkValid(next));
					return next;
				}
				return 0;
			}
			MemBlock* reorganize_freeblock(MemBlock* blk)
			{
				if (!blk)
				{
					return 0;
				}
				assert(blk);
				MemBlock* next = blk->next();
				if(! next->used)
				{
					assert(!check_not_free_block(next));
					//block_set_.erase(block_set_.iterator_to(*next->free_block()));
					erase_free_block(next);
					blk->size += (sizeof(MemBlock) + next->size);
					next->next()->prev = blk;
				}
				if (blk->prev && ! blk->prev->used)
				{
					assert(!check_not_free_block(blk->prev));
					//block_set_.erase(block_set_.iterator_to(*blk->prev->free_block()));
					erase_free_block(blk->prev);
					blk->prev->size += sizeof(MemBlock) + blk->size;
					blk->next()->prev = blk->prev;
					blk = blk->prev;
				}
				return blk;
			}
			void add_free_block_from_free(MemBlock* blk){
				if (blk)
				{
					blk->used = false;
					if( cached_block_ == 0)
					{
						cached_block_ = reorganize_freeblock(blk);
						assert(checkValid(cached_block_));
					}
					else if (!cached_block_->prev && !cached_block_->next()->size)//this is a whole block allocated
					{
						//back to system ? 
						//back to tree ?
						insert_free_block(cached_block_);
						cached_block_ = reorganize_freeblock(blk);
						assert(checkValid(cached_block_));
					}
					else
					{
						//this->free(cached_mr_block_+1);
#ifdef _DEBUG
						if (cached_block_->prev && cached_block_->prev != blk)
						{
							assert(cached_block_->prev->used);
						}
						assert(cached_block_->next()== blk || cached_block_->next()->used);
#endif
						
						if (blk->prev == cached_block_)
						{
							cached_block_->size += (sizeof(MemBlock) + blk->size);
							blk->next()->prev = cached_block_;
							cached_block_  = reorganize_freeblock(cached_block_);
							assert(checkValid(cached_block_));
							assert(cached_block_->next()->used && (!cached_block_->prev || cached_block_->prev->used));
							return;
						}
						else if (cached_block_->prev == blk)
						{
							blk->size += (sizeof(MemBlock) + cached_block_->size);
							cached_block_->next()->prev = blk;
							cached_block_ = blk;
							cached_block_ = reorganize_freeblock(cached_block_);
							assert(checkValid(cached_block_));
							assert(cached_block_->next()->used && (!cached_block_->prev || cached_block_->prev->used));
							return;
						}
#ifdef _DEBUG
						if (blk->prev && !blk->prev->used)
						{
							assert(!check_not_free_block(blk->prev));
						}
						if (blk->next() && !blk->next()->used)
						{
							assert(!check_not_free_block(blk->next()));
						}
#endif
						blk = reorganize_freeblock(blk);
						if (blk->prev == cached_block_)
						{
							cached_block_->size += (sizeof(MemBlock) + blk->size);
							blk->next()->prev = cached_block_;
							cached_block_  = reorganize_freeblock(cached_block_);
							assert(checkValid(cached_block_));
							assert(cached_block_->next()->used && (!cached_block_->prev || cached_block_->prev->used));
							return;
						}
						else if (cached_block_->prev == blk)
						{
							blk->size += (sizeof(MemBlock) + cached_block_->size);
							cached_block_->next()->prev = blk;
							cached_block_ = blk;
							cached_block_ = reorganize_freeblock(cached_block_);
							assert(checkValid(cached_block_));
							assert(cached_block_->next()->used && (!cached_block_->prev || cached_block_->prev->used));
							return;
						}
						if (blk->size > cached_block_->size)
						{
							insert_free_block(cached_block_);
							cached_block_ = blk;
							assert(checkValid(cached_block_));
						}
						else
						{
							insert_free_block(blk);
							assert(!check_not_free_block(blk));
						}
					}
				}
			}
			void add_free_block(MemBlock* blk){
				if (blk)
				{
					blk->used = false;
#ifdef _DEBUG
					if (blk->prev && blk->prev != cached_block_)
					{
						assert(blk->prev->used);
					}
					assert(blk->next()== cached_block_ || blk->next()->used);
					if((intptr_t)blk == 0x05479e68)
						int take_breka= 0;
#endif
					if( cached_block_ == 0)
					{	
						cached_block_ = blk;
						assert(checkValid(cached_block_));
					}
					else if (!cached_block_->prev && !cached_block_->next()->size)//this is a whole block allocated
					{
						//back to system ? 
						//back to tree ?
						insert_free_block(cached_block_);
						cached_block_ = blk;
						assert(checkValid(cached_block_));
					}
					else
					{
						//this->free(cached_mr_block_+1);
						if (blk->prev == cached_block_)
						{
							cached_block_->size += (sizeof(MemBlock) + blk->size);
							blk->next()->prev = cached_block_;
							assert(cached_block_->next()->used && (!blk->prev || blk->prev->used));
							return;
						}
						if (cached_block_->prev == blk)
						{
							blk->size += (sizeof(MemBlock) + cached_block_->size);
							cached_block_->next()->prev = blk;
							cached_block_ = blk;
							assert(checkValid(cached_block_));
							assert(cached_block_->next()->used && (!blk->prev || blk->prev->used));
							return;
						}

						assert(blk->next()->used && (!blk->prev || blk->prev->used));
						assert(cached_block_->next()->used && (!blk->prev || blk->prev->used));
						if (cached_block_->size > blk->size)
						{
							//block_set_.insert(*blk->free_block());
							insert_free_block(blk);
							return;
						}
						else
						{
							//block_set_.insert(*cached_mr_block_->free_block());
							insert_free_block(cached_block_);
							cached_block_ = blk;
							assert(checkValid(cached_block_));
						}

					}
				}
			}

		};
		template<typename Mutex>
		inline void* big_memory_pool<Mutex>::alloc(size_t sz)
		{
			return tree_alloc(sz);
		}
		template<typename Mutex>
		inline void* big_memory_pool<Mutex>::tree_alloc(size_t sz)//0
		{
			//
			scope_lock_t lock(mutex_);
			debug_allocated_size += sz;
			assert(checkAllBlocks());
			if(sz < sizeof(FreeBlock)) sz = sizeof(FreeBlock);
			sz = details::round_up(sz,sizeof(MemBlock));

			if (cached_block_){
				assert(checkValid(cached_block_));
				MemBlock* cached_next = split_block(cached_block_,sz);
				if (cached_next)
				{
					MemBlock* ret = cached_block_;
					cached_block_ = cached_next;
					ret->used = true;
					cached_block_->used = false;
					assert(checkAllBlocks());
					return ret+1;
				}
			}
			if (mr_block_)
			{
				assert(checkValid(mr_block_));
				MemBlock* mr_next = split_block(mr_block_,sz);
				if (mr_next)
				{
					MemBlock* ret = mr_block_;
					ret->used = true;
					mr_block_ = mr_next;
					mr_block_->used = false;
					assert(checkAllBlocks());
					return ret+1;
				}
			}
			debug_find_count ++;

			//static char buffer[sizeof(MemBlock) + sizeof(FreeBlock)]={0};
			//MemBlock* tmp_memblock = new (buffer) MemBlock(0,sz);
			//FreeBlock* search_value = reinterpret_cast<FreeBlock*>(tmp_memblock+1);
#define BEST_FIT
#ifdef BEST_FIT
			block_set_t::iterator it = block_set_.lower_bound(sz);//best fit
#else
			block_set_t::iterator it = block_set_.end();// this is log N operation here
			if(!block_set_.empty()) --it;
#endif
#ifdef BEST_FIT
			if (it != block_set_.end())
#else
			if(it != block_set_.end() && (*it).getMemBlock()->size >= sz)
#endif
			{
				debug_found_count ++;
				
				FreeBlock* free_block = it->next(); // improves removal time
				MemBlock* m = free_block->getMemBlock();

				//if(m == mr_block_)
				//	mr_block_ = 0;
				//else
					block_set_.erase(free_block);

				MemBlock* next = split_block(m,sz);
				m->used = true;
				
#ifdef BEST_FIT
				add_free_block(next);
#else
				add_free_block(next);
#endif
				assert(checkAllBlocks());
				return m+1;
			}
			//allocate from raw memory
			//if not found
			std::size_t malloc_size = details::round_up(sz+2*sizeof(MemBlock),details::PAGE_SIZE);

			debug_total_size+=malloc_size;

			MemBlock* block = reinterpret_cast<MemBlock*>(details::virtual_alloc(malloc_size));

			if (!block)//malloc failed
			{
				//try to return the memory back to OS and re-malloc
				if(!release_memory())
					return 0;
				block = reinterpret_cast<MemBlock*>(details::virtual_alloc(malloc_size));
				if( !block) return 0;
			}
			block->prev = 0;
			block->size = MemBlock::size_type(malloc_size - 2 * sizeof(MemBlock));
			block->used = true;//return this block
			//split this block if possible
			if(malloc_size > (sz+sizeof(MemBlock) * 3 + sizeof(FreeBlock)))
			{
				MemBlock* next = (MemBlock*)((char*)(block + 1) + sz);
				next->prev = block;
				next->size = MemBlock::size_type(malloc_size-sz-3*sizeof(MemBlock) - sizeof(FreeBlock));
				block->size = MemBlock::size_type(sz);
				//block_set_.insert(next);

				//add tail
				MemBlock* tail = (MemBlock*)((char*)(next + 1) + next->size);
				tail->size = 0;
				tail->prev = next;
				tail->used = true;//zero size
				add_free_block(next);
			}
			else
			{
				//add tail
				MemBlock* tail = (MemBlock*)((char*)(block + 1) + block->size);
				tail->size = 0;
				tail->prev = block;
				tail->used = true;
			}
			assert(checkAllBlocks());
			return block+1;
		}
		template<typename Mutex>
		inline void big_memory_pool<Mutex>::tree_free(void* p)
		{
			//if(p)
			{
				scope_lock_t lock(mutex_);
				MemBlock* cur = static_cast<MemBlock*>(p) - 1;
				if((intptr_t)cur == 0x05479e68)
					assert(checkValid(cur));
				add_free_block_from_free(cur);
			}
			assert(checkAllBlocks());
		}
		template<typename Mutex>
		inline void* big_memory_pool<Mutex>::tree_realloc(void* ptr,size_t sz){
			scope_lock_t lock(mutex_);

			if (sz < sizeof(FreeBlock))
				sz = sizeof(FreeBlock);
			sz = details::round_up(sz,sizeof(MemBlock));
			MemBlock* cur = static_cast<MemBlock*>(ptr) - 1;
			assert(checkAllBlocks());
			//the size is shrink
			if (cur->size >= sz )
			{
				//if(cur->size > sz + sizeof(MemBlock))
				{
					add_free_block_from_free(split_block(cur,sz));
				}
				assert(cur->size >= sz);
				assert(checkAllBlocks());
				return cur+1;
			}


			//search for next free page
			MemBlock* next = cur->next();
			MemBlock* prev = cur->prev;

			size_t next_merged_size = next->used? 0 : next->size + sizeof(MemBlock);
			if (cur->size + next_merged_size >= sz)
			{
				if(cached_block_ == next) cached_block_ = 0;
				else {
					assert(!check_not_free_block(next));
					//block_set_.erase(block_set_.iterator_to(*next->free_block()));
					erase_free_block(next);
				}

				assert(next->next()->used || next->used);
				next->next()->prev = cur;
				cur->size += next_merged_size;
				add_free_block(split_block(cur,sz)) ;
				assert(checkAllBlocks());
				return cur+1;
			}
			//search for prev free page
			if(prev && !prev->used && 
				prev->size +cur->size +  sizeof(MemBlock) + next_merged_size >= sz)
			{
				//merge with next and previous
				if(next_merged_size){
					if(cached_block_ == next)
						cached_block_ = 0;
					else {
						assert(!check_not_free_block(next) );
						//block_set_.erase(block_set_.iterator_to(*next->free_block()));
						erase_free_block(next);
						next->next()->prev = prev;
					}
				}
				if(cached_block_ == prev)
					cached_block_ = 0;
				else {
					assert(!check_not_free_block(prev) );
					//block_set_.erase(block_set_.iterator_to(*prev->free_block()));
					erase_free_block(prev);
				}

				next->prev = prev;
				prev->size = prev->size + cur->size + MemBlock::size_type(sizeof(MemBlock) + next_merged_size);
				::memmove(prev+1,cur+1,cur->size);
				prev->used = true;
				add_free_block(split_block(prev,sz));

				assert(checkAllBlocks());

				return prev+1;
			}
			//alloc and copy and free it up
			void* new_ptr = this->alloc(sz);
			if (new_ptr)
			{
				::memcpy(new_ptr,ptr,cur->size);
				this->free(ptr);
				return new_ptr;
			}

			return 0;
		}
		template<typename Mutex>
		inline bool big_memory_pool<Mutex>::tree_recycle(size_t mem_size)
		{
			//clean unused memory from the free blocks
			scope_lock_t lock(mutex_);
			MemBlock* cached_free_block = reorganize_freeblock(cached_block_);
			if(cached_free_block) {insert_free_block(cached_free_block);cached_block_ = 0;}

			bool ret=false;
			for (block_set_t::iterator bsb_it = block_set_.begin();
				bsb_it != block_set_.end();
				)
			{
				MemBlock* free_block = (*bsb_it).getMemBlock();
				if((free_block)->prev == 0
					&& free_block->next()->size == 0)
				{
					//std::cerr<<(*bsb_it)->size<<std::endl;
					if (mem_size < free_block->size)
					{				
						void* mem = free_block;
						block_set_.erase(bsb_it++);
						details::virtual_free((char* )mem,free_block->size);
						return true;
					}
					mem_size -= free_block->size;

					void* mem = free_block;
					block_set_.erase(bsb_it++);
					details::virtual_free((char* )mem,free_block->size);
					ret=true;
				}
				else
					++bsb_it;
			}
			//assert(free_blocks.empty()? true : ret);
			return ret;
		}
		template<typename Mutex>
		class small_memory_pool
		{
		private:
			// minimum allocation size, must be a power of two
			// and it needs to be able to fit a pointer
			static const size_t MIN_ALLOCATION_LOG2 = 3UL;
			static const size_t MIN_ALLOCATION  = 1UL << MIN_ALLOCATION_LOG2;

			// the maximum small allocation, anything larger goes to the tree allocator
			// must be a power of two
			static const size_t MAX_SMALL_ALLOCATION_LOG2 = 8UL;
			static const size_t MAX_SMALL_ALLOCATION  = 1UL << MAX_SMALL_ALLOCATION_LOG2;

			// default alignment, must be a power of two
			static const size_t DEFAULT_ALIGNMENT  = sizeof(double);

			//// page size controls the size of pages we get from the OS
			//// virtual memory is not necessary, on Win32 it's just convenient
			static const size_t PAGE_SIZE_LOG2  = details::VIRTUAL_PAGE_SIZE_LOG2;
			static const size_t PAGE_SIZE  = 1UL << PAGE_SIZE_LOG2;

			static const size_t NUM_BUCKETS  = (MAX_SMALL_ALLOCATION / MIN_ALLOCATION);



			// bucket spacing functions control how the size-space is divided between buckets
			// currently we use linear spacing, could be changed to logarithmic etc
			static inline unsigned bucket_spacing_function(size_t size) { 
				return (unsigned)((size + (MIN_ALLOCATION-1)) >> MIN_ALLOCATION_LOG2) - 1;
			}
			static inline unsigned bucket_spacing_function_aligned(size_t size) { 
				return (unsigned)(size >> MIN_ALLOCATION_LOG2) - 1;
			}
			static inline size_t bucket_spacing_function_inverse(unsigned index) { 
				return (size_t)(index + 1) << MIN_ALLOCATION_LOG2;
			}

			// the page structure is where the small allocator stores all its book-keeping information
			// it is always located at the back of a OS page 
			struct free_link {
				free_link* mNext;
			};
			struct page : intrusive_list<page>::node {
				page(free_link* freeList, size_t elemSize, unsigned marker) 
					: mFreeList(freeList), mBucketIndex((unsigned short)bucket_spacing_function_aligned(elemSize)), mUseCount(0) {
						mMarker = marker ^ (unsigned)((size_t)this); 
				}
				free_link* mFreeList;
				unsigned short mBucketIndex;
				unsigned short mUseCount;
				unsigned mMarker;
				size_t elem_size() const {return bucket_spacing_function_inverse(mBucketIndex);}
				unsigned bucket_index() const {return mBucketIndex;}
				size_t count() const {return mUseCount;}
				bool empty() const {return mUseCount == 0;}
				void inc_ref() {mUseCount++;}
				void dec_ref() {assert(mUseCount > 0); mUseCount--;}
				bool check_marker(unsigned marker) const {return mMarker == (marker ^ (unsigned)((size_t)this));}
			};
			typedef intrusive_list<page> page_list;
			typedef details::scope_lock<Mutex> scope_lock_t;
			class bucket:Mutex {
				page_list mPageList;
//#ifdef MULTITHREADED
				
				//static const size_t SPIN_COUNT = 256;
//				mutable mutex mLock;
//#endif
				unsigned mMarker;
//#ifdef MULTITHREADED
//				unsigned char _padding[sizeof(void*)*16 - sizeof(page_list) - sizeof(mutex) - sizeof(unsigned)];
//#else
//				unsigned char _padding[sizeof(void*)*4 - sizeof(page_list) - sizeof(unsigned)];
//#endif
				static const unsigned MARKER = 0x628bf2b6;
			public:
				bucket()
				{
					// generate a random marker to be hashed with the page info address
					// on Win32 rand max is merely 16 bit so we use two random values
#if (RAND_MAX <= SHRT_MAX)
					mMarker = (rand()*(RAND_MAX+1) + rand()) ^ MARKER;
#else
					mMarker = rand() ^ MARKER;
#endif
				}
//#ifdef MULTITHREADED
				Mutex& get_lock() const {return const_cast<bucket&>(*this);}
//#endif
				unsigned marker() const {return mMarker;}
				const page* page_list_begin() const {return mPageList.begin();}
				page* page_list_begin() {return mPageList.begin();}
				const page* page_list_end() const {return mPageList.end();}
				page* page_list_end() {return mPageList.end();}
				bool page_list_empty() const {return mPageList.empty();}
				void add_free_page(page* p) {mPageList.push_front(p);}
				page* get_free_page(){
					if (!mPageList.empty()) {
						page* p = &mPageList.front();
						if (p->mFreeList)
							return p;
					}
					return NULL;
				}

				void* alloc(page* p) {
					// get an element from the free list
					assert(p && p->mFreeList);
					p->inc_ref();
					free_link* free = p->mFreeList;
					free_link* next = free->mNext;
					p->mFreeList = next;
					if (!next) {
						// if full, auto sort to back
						p->unlink();
						mPageList.push_back(p);
					}
					return (void*)free;
				}
				void free(page* p, void* ptr){
					// add the element back to the free list
					free_link* free_list = p->mFreeList;
					free_link* lnk = (free_link*)ptr;
					lnk->mNext = free_list;
					p->mFreeList = lnk;
					p->dec_ref();
					if (!free_list) {
						// if the page was previously full, auto sort to front
						p->unlink();
						mPageList.push_front(p);
					}
				}
			};
			void* bucket_system_alloc(){	
				void* ptr = details::virtual_alloc(details::PAGE_SIZE);
				// page address must be naturally aligned
				assert(((size_t)ptr & (details::PAGE_SIZE-1)) == 0);
				return ptr;
			}
			void bucket_system_free(void* ptr){
				assert(ptr);
				details::virtual_free(ptr,PAGE_SIZE);
			}
			page* bucket_grow(size_t elemSize, unsigned marker)
			{
				// make sure mUseCount won't overflow
				assert((details::PAGE_SIZE-sizeof(page))/elemSize <= std::numeric_limits<unsigned short>::max());
				if (void* mem = bucket_system_alloc()) {
					// build the free list inside the new page
					// the page info sits at the end of the page
					// most of the time it hides in the alignment remainder and ends up costing no memory
					size_t i = 0;
					size_t n = ((details::PAGE_SIZE-sizeof(page))/elemSize)*elemSize;
					for (; i < n-elemSize; i += elemSize)
						((free_link*)((char*)mem + i))->mNext = (free_link*)((char*)mem + i + elemSize);
					((free_link*)((char*)mem + i))->mNext = NULL;
					assert(i + elemSize + sizeof(page) <= details::PAGE_SIZE);
					page* p = ptr_get_page(mem);
					new ((void*)p) page((free_link*)mem, (unsigned short)elemSize, marker);
					return p;
				}
				return NULL;
			}
			void* bucket_alloc(size_t size){
				assert(size <= MAX_SMALL_ALLOCATION);
				unsigned bi = bucket_spacing_function(size);
				assert(bi < NUM_BUCKETS);
//#ifdef MULTITHREADED
				scope_lock_t lock(mBuckets[bi].get_lock());
//#endif
				// get the page info and check if there's any available elements
				page* p = mBuckets[bi].get_free_page();
				if (!p) {
					// get a page from the OS, initialize it and add it to the list
					size_t bsize = bucket_spacing_function_inverse(bi);
					p = bucket_grow(bsize, mBuckets[bi].marker());
					if (!p)
						return NULL;
					mBuckets[bi].add_free_page(p);
				}
				assert(p->elem_size() >= size);
				return mBuckets[bi].alloc(p);
			}
			void* bucket_alloc_direct(unsigned bi)
			{
				assert(bi < NUM_BUCKETS);
//#ifdef MULTITHREADED
				scope_lock_t lock(mBuckets[bi].get_lock());
//#endif
				page* p = mBuckets[bi].get_free_page();
				if (!p) {
					size_t bsize = bucket_spacing_function_inverse(bi);
					p = bucket_grow(bsize, mBuckets[bi].marker());
					if (!p)
						return NULL;
					mBuckets[bi].add_free_page(p);
				}
				return mBuckets[bi].alloc(p);
			}
			void* bucket_realloc(void* ptr, size_t size)
			{
				page* p = ptr_get_page(ptr);
				size_t elemSize = p->elem_size();
				if (size <= elemSize)
					return ptr;		
				void* newPtr = bucket_alloc(size);
				if (!newPtr)
					return NULL;
				memcpy(newPtr, ptr, elemSize );
				bucket_free(ptr);
				return newPtr;
			}
			void bucket_free(void* ptr)
			{
				page* p = ptr_get_page(ptr);
				unsigned bi = p->bucket_index();
				assert(bi < NUM_BUCKETS);
//#ifdef MULTITHREADED
				scope_lock_t lock(mBuckets[bi].get_lock());
//#endif
				mBuckets[bi].free(p, ptr);
			}
			void bucket_free_direct(void* ptr, unsigned bi)
			{
				assert(bi < NUM_BUCKETS);
				page* p = ptr_get_page(ptr);
				// if this asserts, the free size doesn't match the allocated size
				// most likely a class needs a base virtual destructor
				assert(bi == p->bucket_index());
//#ifdef MULTITHREADED
				scope_lock_t lock(mBuckets[bi].get_lock());
//#endif
				mBuckets[bi].free(p, ptr);
			}
			bool bucket_purge()
			{
				bool ret = false;
				for (unsigned i = 0; i < NUM_BUCKETS; i++) {
//#ifdef MULTITHREADED
					scope_lock_t lock(mBuckets[i].get_lock());
//#endif
					page *pageEnd = mBuckets[i].page_list_end();
					for (page* p = mBuckets[i].page_list_begin(); p != pageEnd; ) {
						// early out if we reach fully occupied page (the remaining should all be full)
						if (p->mFreeList == NULL) 
							break;
						page* next = p->next();
						if (p->empty()) {
							assert(p->mFreeList);
							p->unlink();
							void* memAddr = details::align_down((char*)p, details::PAGE_SIZE);
							bucket_system_free(memAddr);
							ret = true;
						}
						p = next;
					}
				}
				return ret;

			}

			// locate the page information from a pointer
			static inline page* ptr_get_page(void* ptr) {
				return (page*)(details::align_down((char*)ptr, details::PAGE_SIZE) + (details::PAGE_SIZE - sizeof(page)));
			}

			bool ptr_in_bucket(void* ptr) const {
				bool result = false;
				page* p = ptr_get_page(ptr);
				unsigned bi = p->bucket_index();
				if (bi < NUM_BUCKETS) {
					result = p->check_marker(mBuckets[bi].marker());
					// there's a minimal chance the marker check is not sufficient
					// due to random data that happens to match the marker
					// the exhaustive search below will catch this case 
					// and that will indicate that more secure measures are needed
#ifndef NDEBUG
//#ifdef MULTITHREADED
					scope_lock_t lock(mBuckets[bi].get_lock());
//#endif
					const page* pe = mBuckets[bi].page_list_end();
					const page* pb = mBuckets[bi].page_list_begin();
					for (; pb != pe && pb != p; pb = pb->next()) {}
					assert(result == (pb == p));
#endif
				}
				return result;
			}

			bucket mBuckets[NUM_BUCKETS];
		public:
			void* alloc(size_t sz){
				sz = clamp_small_allocation(sz);
				return  bucket_alloc_direct(bucket_spacing_function(sz ));
			}
			void* realloc(void* ptr,size_t sz){				
				void* newPtr = bucket_realloc(ptr, sz );
				return newPtr;
			}
			void free(void* p){bucket_free(p);}
			void free(void* ptr,size_t sz){
				// if this asserts probably the original alloc used alignment
				assert(ptr_in_bucket(ptr));
				return bucket_free_direct(ptr, bucket_spacing_function(sz ));
			}
			bool release_memory(){return bucket_purge();}
			bool is_small_allocation(void* ptr){return ptr_in_bucket(ptr);}
			static inline bool is_small_allocation(size_t s) {
				return s  <= MAX_SMALL_ALLOCATION;
			}
			static inline size_t clamp_small_allocation(size_t s) {
				return (s  < MIN_ALLOCATION) ? MIN_ALLOCATION  : s;
			}
			size_t size_of(void* ptr){
				assert(is_small_allocation(ptr));
				return ptr_get_page(ptr)->elem_size();
			};
		protected:
		private:
		};
	}
	//template<typename ThreadingModel,bool Auto = false,typename UserTag = details::memory_null_tag>
	template<typename Mutex>
	class memory_pool{
	public:
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
	public:
		void* alloc(size_t sz);
		void* calloc(size_t sz);
		void* realloc(void* ptr,size_t sz);
		void free(void* p);
		bool release_memory(size_t mem_size = -1)
		{
			//printf("big memory allocator allocate :%d \n",big_memory_allocator_.debug_total_size);
			//small
			bool s = small_memory_allocator_.release_memory();
			bool b = big_memory_allocator_.release_memory(mem_size);
			return s || b;
		}
		void free(void* ptr,size_t sz)
		{
			if (small_memory_allocator_.is_small_allocation(sz)) {
				return small_memory_allocator_.free(ptr,sz);
			}
			big_memory_allocator_.free(ptr);
		}
	//private:
		core::big_memory_pool<Mutex> big_memory_allocator_;
		core::small_memory_pool<Mutex> small_memory_allocator_;
	};
	template<typename Mutex>
	inline void* memory_pool<Mutex>::alloc(size_t sz)
	{
		if (!sz )return 0;

		if (small_memory_allocator_.is_small_allocation(sz))
		{
			return small_memory_allocator_.alloc(sz);
		}
		return big_memory_allocator_.alloc(sz);
	}
	template<typename Mutex>
	inline void* memory_pool<Mutex>::calloc(size_t sz)
	{
		void *p = alloc(sz);
		if (p)
		{
			::memset(p,0,sz);
		}
		return p;
	}
	template<typename Mutex>
	inline void* memory_pool<Mutex>::realloc(void* ptr,size_t size)
	{
		if(! ptr)return alloc(size);
		if(! size){ free(ptr); return 0;}

		if (small_memory_allocator_.is_small_allocation(ptr)) {
			size = small_memory_allocator_.clamp_small_allocation(size);
			if (small_memory_allocator_.is_small_allocation(size)) {
				return small_memory_allocator_.realloc(ptr,size);
			}
			void* newPtr = big_memory_allocator_.alloc(size);
			if (!newPtr)
				return 0;
			::memcpy(newPtr, ptr, small_memory_allocator_.size_of(ptr));
			small_memory_allocator_.free(ptr);
			return newPtr;
		}

		return big_memory_allocator_.realloc(ptr,size);
	}
	template<typename Mutex>
	inline void memory_pool<Mutex>::free(void *p)
	{
		if (p)
		{
			if (small_memory_allocator_.is_small_allocation(p))
				return small_memory_allocator_.free(p);
			big_memory_allocator_.free(p);
		}
	}
}

#ifdef _MSC_VER
//msvc compiler
#pragma warning(pop)
#endif
#endif
