#ifndef MABIGOBJECTPOOL_H
#define MABIGOBJECTPOOL_H

#include <set>
#include <boost/pool/pool_alloc.hpp>
#include <boost/preprocessor.hpp>

//implemented by lzy for big object memory allocation (refenrence to game programming gems 7)
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
		}

		template<typename MemAllocator = ma_detail::default_mem_allocator_new_delete>
		struct MABigObjectPool{
			//static std::size_t smallest_size = 256;

			static void* malloc(std::size_t sz);
			static void free(void* p);//mark as freed
			static bool clean_unused(); // this is very slow: true if really free some memory
			static void defragment();//defragment 

		private:
			typedef std::multiset<ma_detail::MemBlock*,ma_detail::block_less_sz,
				boost::pool_allocator<ma_detail::MemBlock,
				boost::default_user_allocator_new_delete,
				boost::details::pool::null_mutex> 
			> BlockSetBySize;

			static BlockSetBySize free_blocks;
		};
		template<typename MemAllocator>
		typename MABigObjectPool<MemAllocator>::BlockSetBySize MABigObjectPool<MemAllocator>::free_blocks;


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
				/*	1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,
				2*8192,4*8192,8*8192,16*8192,32*8192,64*8192,128*8192,256*8192,512*8192,1024*8192,2048*8192,4096*8192,8192*8192,
				8192*8192*2,8192*8192*4,8192*8192*8,8192*8192*16,8192*8192*32,8192*8192*64*/
			};//32 bit
#undef MA_MEMORY_POOL_POOL_SIZES
#undef MA_MEMORY_POOL_GENERATE_POWER2


#ifndef MA_64BIT
#undef MA_ARCH_BIT 
#else 
#undef MA_ARCH_BIT 
#endif
		}
		template<typename MemAllocator>
		inline void MABigObjectPool<MemAllocator>::defragment()
		{
			using namespace ma_detail;
		}
		template<typename MemAllocator>
		inline void* MABigObjectPool<MemAllocator>::malloc(std::size_t sz)
		{
			using namespace ma_detail;
			

			//is it in the free block set
			
			BlockSetBySize::iterator it = free_blocks.lower_bound(& MemBlock(0,sz));
			if (it != free_blocks.end())
			{
				MemBlock* m = (*it);

				free_blocks.erase(it); //erase it before we change it
				if(m->size > sz+sizeof(MemBlock))
				{
					char* next = reinterpret_cast<char*>(m+1) + sz;
					(reinterpret_cast<MemBlock*>(next))->prev = m;
					(reinterpret_cast<MemBlock*>(next))->size = m->size- (sz + sizeof(MemBlock)); 
					m->size = sz;
					assert(reinterpret_cast<MemBlock*>(next)->size && (reinterpret_cast<char*> (m+1) + m->size) == next);
					free_blocks.insert(reinterpret_cast<MemBlock*>(next));
				}

				

				assert(m->size);
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
				if(!clean_unused() || !(block = reinterpret_cast<MemBlock*>(MemAllocator::malloc(malloc_size))))
					return 0;
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

				assert(next->size);
				free_blocks.insert(next);

				//add tail
				MemBlock* tail = (MemBlock*)((char*)(next + 1) + next->size);
				tail->size = 0;
				tail->prev = next;
			}
			else
			{
				MemBlock* tail = (MemBlock*)((char*)(block + 1) + block->size);
				tail->size = 0;
				tail->prev = block;
			}
			assert(block->size);
			return block+1;
		}

		//p must be allocated by this pool 
		//otherwise YOU would be responsible for global warming , the wars and my bad mood, etc. 
		//Even worse doing that may cause The Doom of The Universe !
		template<typename MemAllocator>
		inline void MABigObjectPool<MemAllocator>::free(void* p)
		{
			using namespace ma_detail;

			if(p)
			{
				MemBlock* cur = static_cast<MemBlock*>(p) - 1;
				if(cur->prev)//merge
				{
					BlockSetBySize::iterator start = free_blocks.lower_bound(cur->prev);
					BlockSetBySize::iterator end = free_blocks.upper_bound(cur->prev);
					for (;start != end;++start)
					{
						if (cur->prev == (*start))
						{
							assert(cur->prev->size == (*start)->size);
							free_blocks.erase(start);
						
							MemBlock* next = reinterpret_cast<MemBlock*>( reinterpret_cast<char*>(cur+1) + cur->size);
							next->prev = cur->prev;
						
							cur->prev->size += (sizeof(MemBlock)+cur->size);
							assert(cur->prev->size);
							free_blocks.insert(cur->prev);
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

							free_blocks.erase(start);
							MemBlock* next_next = reinterpret_cast<MemBlock*>( reinterpret_cast<char*>(next+1) + next->size);
							next_next->prev = cur;

							
							cur->size += (sizeof(MemBlock)+next->size);

							assert((char*)(cur + 1) + cur->size == (char*)next_next);
							
							break;
						}
					}

					free_blocks.insert(cur);
				}
			}

		}
		template<typename MemAllocator>
		inline bool MABigObjectPool<MemAllocator>::clean_unused()
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
					void* mem = *bsb_it;
					free_blocks.erase(bsb_it++);
					MemAllocator::free((char* )mem);
				}
				else
					++bsb_it;
			}
			//assert(free_blocks.empty()? true : ret);
			
			return free_blocks.size()<prev_sz;
		}

	}
}
#endif