#ifndef MABIGPOOL_H
#define MABIGPOOL_H

#include <set>
#include <boost/pool/pool_alloc.hpp>
#include <boost/preprocessor.hpp>


#include <map>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

//implemented by lzy for big object memory allocation (reference to game programming gems 7)
//GC won't be in c++0x but pooled memory management is a transparent GC implementation as
//suggested in TR
namespace ma{
	namespace core
	{
		namespace ma_detail{
			struct MemBlock{
				MemBlock(MemBlock* previous,std::size_t sz):prev(previous),size(sz){}
				MemBlock* prev; // point to previous
				std::size_t size;
			};
			struct block_less_sz:std::binary_function<MemBlock*,MemBlock*,bool>
			{
				bool operator()(const MemBlock* lhs,const MemBlock* rhs)const
				{
					return lhs->size < rhs->size;
				}
			};

			struct default_mem_allocator_new_delete{
				static char* malloc(std::size_t size){return new (std::nothrow) char[size];};
				static void free(char * const p){	return delete [](p);}
			};
			struct default_mem_allocator_malloc_free{
				static char* malloc(std::size_t size){return malloc(size);};
				static void free(void *p){	return free(p);}
			};

			struct BigMemPoolDefaultTag{};
			struct BigMemPoolHashedTag{};
		}

		template<typename MemAllocator = ma_detail::default_mem_allocator_new_delete , typename PoolTag = ma_detail::BigMemPoolDefaultTag>
		struct MABigMemoryPool{
			//static std::size_t smallest_size = 256;

			static void* malloc(std::size_t sz); //malloc sz bytes
			static void free(void* p);//mark as freed

			//free mem_size bytes if possible:depend on mem_size and the free_blocks
			// mem_size=-1 means free all
			// this could be very slow: true if really free some memory or free mem_size bytes successfully
			static bool clean_unused(std::size_t mem_size = std::size_t(-1));


#ifdef _DEBUG
			static bool checkValid(ma_detail::MemBlock* block)
			{
				assert(block);
				if(block->prev)
					assert(reinterpret_cast<char*>(block) == (char*)(block->prev+1)+block->prev->size);
				return block->size;
			}

			static bool checkAllBlocks(){
				bool ret = true;
				for(BlockSetBySize::iterator it = free_blocks.begin();it != free_blocks.end(); ++it)
				{
					assert( (ret = checkValid(*it)));
				}
				return ret;
			}
#endif
		private:
			typedef std::multiset<ma_detail::MemBlock*,ma_detail::block_less_sz,
				boost::pool_allocator<ma_detail::MemBlock,
				boost::default_user_allocator_new_delete,
				boost::details::pool::null_mutex> 
			> BlockSetBySize;

			static BlockSetBySize free_blocks;
		};
		template<typename MemAllocator,typename PoolTag>
		typename MABigMemoryPool<MemAllocator,PoolTag>::BlockSetBySize MABigMemoryPool<MemAllocator,PoolTag>::free_blocks;


		namespace ma_detail
		{
#ifndef MA_64BIT
#define MA_ARCH_BIT 32
#else 
#define MA_ARCH_BIT 64
#endif
			//this meta-function is only for this Memory pool
			template<std::size_t N>
			struct power2{
				static const std::size_t value = N < (MA_ARCH_BIT - 1) ? (1 << N) :((std::size_t(-1)) >> 1);
			};
			template<std::size_t N>
			struct log2{
				static const std::size_t value = (log2< (N>>1) >::value) + 1;
			};
			template<>
			struct log2<1>{
				static const std::size_t value = 0;
			};

#define MA_MEMORY_POOL_GENERATE_POWER2(z, N , data)\
	power2<N>::value BOOST_PP_COMMA_IF( BOOST_PP_SUB(data, N) )

#define MA_MEMORY_POOL_POOL_SIZES\
	BOOST_PP_REPEAT(MA_ARCH_BIT, MA_MEMORY_POOL_GENERATE_POWER2 , BOOST_PP_SUB(MA_ARCH_BIT, 1) )

			static const std::size_t pool_sizes[]= {
				MA_MEMORY_POOL_POOL_SIZES
			};//32 bit
#undef MA_MEMORY_POOL_POOL_SIZES
#undef MA_MEMORY_POOL_GENERATE_POWER2

#ifndef MA_64BIT
#undef MA_ARCH_BIT 
#else 
#undef MA_ARCH_BIT 
#endif
		}

		template<typename MemAllocator,typename PoolTag>
		inline void* MABigMemoryPool<MemAllocator,PoolTag>::malloc(std::size_t sz)
		{
			using namespace ma_detail;
			
			assert(checkAllBlocks());

			//is it in the free block set
			
			BlockSetBySize::iterator it = free_blocks.lower_bound(& MemBlock(0,sz));
			if (it != free_blocks.end())
			{
				MemBlock* m = (*it);
				assert(checkValid(m));
				assert(checkAllBlocks());

				

				assert(checkAllBlocks());

				if(m->size > sz+sizeof(MemBlock))
				{
					MemBlock* m_next = reinterpret_cast<MemBlock*>(reinterpret_cast<char*>(m+1) + m->size);

					MemBlock* next = reinterpret_cast<MemBlock*>(reinterpret_cast<char*>(m+1) + sz);
					next->prev = m;
					next->size = m->size- (sz + sizeof(MemBlock)); 
					m_next->prev = next;

					m->size = sz;
					assert(next->size && (reinterpret_cast<char*> (m+1) + m->size) == reinterpret_cast<char*>( next));
					assert(checkValid(m));
					assert(checkValid((next)));
					assert(checkAllBlocks());

					free_blocks.insert(it,(next));

					assert(checkAllBlocks());
				}

				free_blocks.erase(it); //erase it before we change it
				

				assert(m->size);
				assert(checkValid(m));
				assert(checkAllBlocks());

				return m+1;

			}
			//if not found

			assert(std::lower_bound(pool_sizes,pool_sizes+ sizeof(pool_sizes)/sizeof(pool_sizes[0]),sz+2*sizeof(MemBlock)) 
				!= pool_sizes+ sizeof(pool_sizes)/sizeof(pool_sizes[0]));
			std::size_t malloc_size = 
				*std::lower_bound(pool_sizes,pool_sizes+sizeof(pool_sizes)/sizeof(pool_sizes[0]), sz+2*sizeof(MemBlock));

			MemBlock* block = reinterpret_cast<MemBlock*>(MemAllocator::malloc(malloc_size));
			
			if (!block)//malloc failed
			{
				//intentional assigned to block
				if(!clean_unused())
					return 0;
				block = reinterpret_cast<MemBlock*>(MemAllocator::malloc(malloc_size));
				if( !block) return 0;
			}
			block->prev = 0;
			block->size = malloc_size - 2 * sizeof(MemBlock);
			//split this block if possible
			if(malloc_size > (sz+sizeof(MemBlock) * 3))
			{
				MemBlock* next = (MemBlock*)((char*)(block + 1) + sz);
				next->prev = block;
				next->size = malloc_size-sz-3*sizeof(MemBlock);
				block->size = sz;

				assert(next->size && checkValid(next));
				free_blocks.insert(next);

				//add tail
				MemBlock* tail = (MemBlock*)((char*)(next + 1) + next->size);
				tail->size = 0;
				tail->prev = next;
			}
			else
			{
				//add tail
				MemBlock* tail = (MemBlock*)((char*)(block + 1) + block->size);
				tail->size = 0;
				tail->prev = block;
			}
			assert(block->size);
			assert(checkValid(block));
			assert(checkAllBlocks());

			return block+1;
		}

		//p must be allocated by this pool 
		//otherwise YOU would be responsible for global warming , the wars and my bad mood, etc. 
		//Even worse doing that may cause The Doom of The Universe !
		template<typename MemAllocator,typename PoolTag>
		inline void MABigMemoryPool<MemAllocator,PoolTag>::free(void* p)
		{
			using namespace ma_detail;

			assert(checkAllBlocks());
			if(p)
			{
				MemBlock* cur = static_cast<MemBlock*>(p) - 1;

				assert(checkValid(cur));
				if(cur->prev)//merge
				{
					BlockSetBySize::iterator start = free_blocks.lower_bound(cur->prev);
					BlockSetBySize::iterator end = free_blocks.upper_bound(cur->prev);
					for (;start != end;++start)
					{
						if (cur->prev == (*start))
						{
							assert(cur->prev->size == (*start)->size);

							MemBlock* next = reinterpret_cast<MemBlock*>( reinterpret_cast<char*>(cur+1) + cur->size);

							assert(reinterpret_cast<char*>(cur) == (char*)(cur->prev+1)+cur->prev->size);
							assert((char*)(cur->prev+1)+cur->prev->size + (sizeof(MemBlock)+cur->size) == (char*)next);
							
							next->prev = cur->prev;
						
							cur->prev->size += (sizeof(MemBlock)+cur->size);
							assert(cur->prev->size && checkValid(cur->prev));

							free_blocks.insert(start,cur->prev);
							//modify cur->prev's size
							free_blocks.erase(start);
							assert((char*)(cur->prev+1)+cur->prev->size == (char*)next);
							break;
						}
					}
				}
				else{
					assert(cur->size);
					//merge with next
					MemBlock* next = reinterpret_cast<MemBlock*>( reinterpret_cast<char*>(cur+1) + cur->size);
					assert(next->prev == cur);
					BlockSetBySize::iterator start = free_blocks.lower_bound(next);
					BlockSetBySize::iterator end = free_blocks.upper_bound(next);
					for (;start != end;++start)
					{
						if (cur == (*start)->prev)
						{
							
							assert(next->size && next->size == (*start)->size && next->prev == (*start)->prev);
							//modify
							free_blocks.erase(start);
							MemBlock* next_next = reinterpret_cast<MemBlock*>( reinterpret_cast<char*>(next+1) + next->size);
							next_next->prev = cur;

							cur->size += (sizeof(MemBlock)+next->size);

							assert((char*)(cur + 1) + cur->size == (char*)next_next);
							
							break;
						}
					}

					assert(checkValid(cur));
					free_blocks.insert(cur);
				}
			}
			assert(checkAllBlocks());

		}
		template<typename MemAllocator,typename PoolTag>
		inline bool MABigMemoryPool<MemAllocator,PoolTag>::clean_unused(std::size_t mem_size)
		{
			using namespace ma_detail;

			//clean unused memory from the free blocks
			std::size_t prev_sz =  free_blocks.size();
			for (BlockSetBySize::iterator bsb_it = free_blocks.begin();
				bsb_it != free_blocks.end();
				)
			{
				if((*bsb_it)->prev == 0
					&& reinterpret_cast<MemBlock*> (reinterpret_cast<char*>(*bsb_it)+(*bsb_it)->size)->size == 0)
				{
					std::cerr<<(*bsb_it)->size<<std::endl;
					if (mem_size < (*bsb_it)->size)
					{				
						void* mem = *bsb_it;
						free_blocks.erase(bsb_it++);
						MemAllocator::free((char* )mem);

						assert(checkAllBlocks());
						return true;
					}
					mem_size -= (*bsb_it)->size;

					void* mem = *bsb_it;
					free_blocks.erase(bsb_it++);
					MemAllocator::free((char* )mem);
				}
				else
					++bsb_it;
			}
			//assert(free_blocks.empty()? true : ret);
			assert(checkAllBlocks());
			return free_blocks.size()<prev_sz;
		}



		//////////////////////////////////////////////////////////////////////////
		//implementation of hashed tag: optimized for the situations like many free blocks with same size

		namespace ma_detail
		{
			
		}
		template<typename MemAllocator>
		struct MABigMemoryPool<MemAllocator,ma_detail::BigMemPoolHashedTag>
		{
			static void* malloc(std::size_t sz); //malloc sz bytes
			static void free(void* p);//mark as freed

			//free mem_size bytes if possible:depend on mem_size and the free_blocks
			// mem_size=-1 means free all
			// this could be very slow: true if really free some memory or free mem_size bytes successfully
			static bool clean_unused(std::size_t mem_size = std::size_t(-1));

#ifdef _DEBUG
			static bool checkValid(ma_detail::MemBlock* block)
			{
				assert(block);
				if(block->prev)
					assert(reinterpret_cast<char*>(block) == (char*)(block->prev+1)+block->prev->size);
				return block->size;
			}

			static bool checkAllBlocks(){
				bool ret = true;
				for(BlockSetBySize::iterator it = free_blocks.begin();it != free_blocks.end(); ++it)
				{
					assert( (ret = ret && checkValid(*it)));
				}
				return ret;
			}
#endif
		private:
			

			typedef std::multiset<ma_detail::MemBlock*,ma_detail::block_less_sz,
				boost::pool_allocator<ma_detail::MemBlock,
				boost::default_user_allocator_new_delete,
				boost::details::pool::null_mutex> 
			> BlockSetBySize;
			typedef std::pair<typename BlockSetBySize::iterator ,bool> InsertedRetType;
			typedef boost::unordered_map<ma_detail::MemBlock*,BlockSetBySize::iterator> 
				Mem2Blocks;
			static BlockSetBySize free_blocks;
			

			static Mem2Blocks free_blocks_poses;
		};
		template<typename MemAllocator>
		typename MABigMemoryPool<MemAllocator,ma_detail::BigMemPoolHashedTag>::BlockSetBySize 
			MABigMemoryPool<MemAllocator,ma_detail::BigMemPoolHashedTag>::free_blocks;
		template<typename MemAllocator>
		typename MABigMemoryPool<MemAllocator,ma_detail::BigMemPoolHashedTag>::Mem2Blocks 
			MABigMemoryPool<MemAllocator,ma_detail::BigMemPoolHashedTag>::free_blocks_poses;


		template<typename MemAllocator>
		inline void* MABigMemoryPool<MemAllocator,ma_detail::BigMemPoolHashedTag>::malloc(std::size_t sz)
		{
			using namespace ma_detail;

			assert(checkAllBlocks());

			//is it in the free block set

			BlockSetBySize::iterator it = free_blocks.lower_bound(& MemBlock(0,sz));
			if (it != free_blocks.end())
			{
				MemBlock* m = (*it);
				assert(checkValid(m));
				assert(checkAllBlocks());



				assert(checkAllBlocks());

				if(m->size > sz+sizeof(MemBlock))
				{
					MemBlock* m_next = reinterpret_cast<MemBlock*>(reinterpret_cast<char*>(m+1) + m->size);

					MemBlock* next = reinterpret_cast<MemBlock*>(reinterpret_cast<char*>(m+1) + sz);
					next->prev = m;
					next->size = m->size- (sz + sizeof(MemBlock)); 
					m_next->prev = next;

					m->size = sz;
					assert(next->size && (reinterpret_cast<char*> (m+1) + m->size) == reinterpret_cast<char*>( next));
					assert(checkValid(m));
					assert(checkValid((next)));
					assert(checkAllBlocks());

					InsertedRetType pos = free_blocks.insert(it,(next));
					if(pos.second)
						free_blocks_poses.insert(std::make_pair(next,pos.first));



					assert(checkAllBlocks());
				}

				free_blocks.erase(it); //erase it before we change it


				assert(m->size);
				assert(checkValid(m));
				assert(checkAllBlocks());

				return m+1;

			}
			//if not found

			assert(std::lower_bound(pool_sizes,pool_sizes+ sizeof(pool_sizes)/sizeof(pool_sizes[0]),sz+2*sizeof(MemBlock)) 
				!= pool_sizes+ sizeof(pool_sizes)/sizeof(pool_sizes[0]));
			std::size_t malloc_size = 
				*std::lower_bound(pool_sizes,pool_sizes+sizeof(pool_sizes)/sizeof(pool_sizes[0]), sz+2*sizeof(MemBlock));

			MemBlock* block = reinterpret_cast<MemBlock*>(MemAllocator::malloc(malloc_size));

			if (!block)//malloc failed
			{
				//intentional assigned to block
				if(!clean_unused())
					return 0;
				block = reinterpret_cast<MemBlock*>(MemAllocator::malloc(malloc_size));
				if( !block) return 0;
			}
			block->prev = 0;
			block->size = malloc_size - 2 * sizeof(MemBlock);
			//split this block if possible
			if(malloc_size > (sz+sizeof(MemBlock) * 3))
			{
				MemBlock* next = (MemBlock*)((char*)(block + 1) + sz);
				next->prev = block;
				next->size = malloc_size-sz-3*sizeof(MemBlock);
				block->size = sz;

				assert(next->size && checkValid(next));
				InsertedRetType pos = free_blocks.insert(next);
				if(pos.second)
					free_blocks_poses.insert(std::make_pair(next,free_blocks_poses.first));

				//add tail
				MemBlock* tail = (MemBlock*)((char*)(next + 1) + next->size);
				tail->size = 0;
				tail->prev = next;
			}
			else
			{
				//add tail
				MemBlock* tail = (MemBlock*)((char*)(block + 1) + block->size);
				tail->size = 0;
				tail->prev = block;
			}
			assert(block->size);
			assert(checkValid(block));
			assert(checkAllBlocks());

			return block+1;
		}

		//p must be allocated by this pool 
		//otherwise YOU would be responsible for global warming , the wars and my bad mood, etc. 
		//Even worse doing that may cause The Doom of The Universe !
		template<typename MemAllocator>
		inline void MABigMemoryPool<MemAllocator,ma_detail::BigMemPoolHashedTag>::free(void* p)
		{
			using namespace ma_detail;

			assert(checkAllBlocks());
			if(p)
			{
				MemBlock* cur = static_cast<MemBlock*>(p) - 1;

				assert(checkValid(cur));
				if(cur->prev)//merge
				{
					BlockSetBySize::iterator start = free_blocks.lower_bound(cur->prev);
					BlockSetBySize::iterator end = free_blocks.upper_bound(cur->prev);
					for (;start != end;++start)
					{
						if (cur->prev == (*start))
						{
							assert(cur->prev->size == (*start)->size);

							MemBlock* next = reinterpret_cast<MemBlock*>( reinterpret_cast<char*>(cur+1) + cur->size);

							assert(reinterpret_cast<char*>(cur) == (char*)(cur->prev+1)+cur->prev->size);
							assert((char*)(cur->prev+1)+cur->prev->size + (sizeof(MemBlock)+cur->size) == (char*)next);

							next->prev = cur->prev;

							cur->prev->size += (sizeof(MemBlock)+cur->size);
							assert(cur->prev->size && checkValid(cur->prev));

							InsertedRetType pos = free_blocks.insert(start,cur->prev);
							if (pos.second)
							{
								free_blocks_poses.insert(std::make_pair(cur->prev,pos.first));
							}
							//modify cur->prev's size
							free_blocks.erase(start);
							assert((char*)(cur->prev+1)+cur->prev->size == (char*)next);
							break;
						}
					}
				}
				else{
					assert(cur->size);
					//merge with next
					MemBlock* next = reinterpret_cast<MemBlock*>( reinterpret_cast<char*>(cur+1) + cur->size);
					assert(next->prev == cur);
					BlockSetBySize::iterator start = free_blocks.lower_bound(next);
					BlockSetBySize::iterator end = free_blocks.upper_bound(next);
					for (;start != end;++start)
					{
						if (cur == (*start)->prev)
						{

							assert(next->size && next->size == (*start)->size && next->prev == (*start)->prev);
							//modify
							free_blocks.erase(start);
							MemBlock* next_next = reinterpret_cast<MemBlock*>( reinterpret_cast<char*>(next+1) + next->size);
							next_next->prev = cur;

							cur->size += (sizeof(MemBlock)+next->size);

							assert((char*)(cur + 1) + cur->size == (char*)next_next);

							break;
						}
					}

					assert(checkValid(cur));
					InsertedRetType pos = free_blocks.insert(cur);
					if (pos.second)
					{
						free_blocks_poses.insert(std::make_pair(cur,pos.first));
					}
				}
			}
			assert(checkAllBlocks());

		}
		template<typename MemAllocator>
		inline bool MABigMemoryPool<MemAllocator,ma_detail::BigMemPoolHashedTag>::clean_unused(std::size_t mem_size)
		{
			using namespace ma_detail;

			//clean unused memory from the free blocks
			std::size_t prev_sz =  free_blocks.size();
			for (BlockSetBySize::iterator bsb_it = free_blocks.begin();
				bsb_it != free_blocks.end();
				)
			{
				if((*bsb_it)->prev == 0
					&& reinterpret_cast<MemBlock*> (reinterpret_cast<char*>(*bsb_it)+(*bsb_it)->size)->size == 0)
				{
					std::cerr<<(*bsb_it)->size<<std::endl;
					if (mem_size < (*bsb_it)->size)
					{				
						void* mem = *bsb_it;
						free_blocks.erase(bsb_it++);
						MemAllocator::free((char* )mem);

						assert(checkAllBlocks());
						return true;
					}
					mem_size -= (*bsb_it)->size;

					void* mem = *bsb_it;
					free_blocks.erase(bsb_it++);
					MemAllocator::free((char* )mem);
				}
				else
					++bsb_it;
			}
			//assert(free_blocks.empty()? true : ret);
			assert(checkAllBlocks());
			return free_blocks.size()<prev_sz;
		}


	}
}
#endif