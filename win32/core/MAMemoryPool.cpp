#include "MAMemoryPool.h"

#include "FSBAllocator.h"

#include <set>
#include <boost/unordered_set.hpp>
#include <boost/pool/singleton_pool.hpp>
#include <boost/pool/pool_alloc.hpp>

#include <boost/preprocessor.hpp>

#ifndef MA_64BIT
#define MA_ARCH_BIT 32
#else 
#define MA_ARCH_BIT 64
#endif
namespace ma
{
namespace core
{
	namespace{
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


		struct block{
			block():start(0),sz(0){}
			block(char* st,std::size_t siz):start(st),sz(siz){}
			char* start;
			std::size_t sz;
		};

		struct block_start_equal:std::binary_function<block,block,bool>
		{
			bool operator()(const block& lhs,const block& rhs)const
			{
				return lhs.start == rhs.start;
			}
		};
		struct block_exact_equal:std::binary_function<block,block,bool>
		{
			bool operator()(const block& lhs,const block& rhs)const
			{
				return lhs.start == rhs.start && lhs.sz == rhs.sz;
			}
		};

		struct block_compare:std::binary_function<block,block,bool>
		{
			bool operator()(const block& lhs,const block& rhs)const{
				return lhs.sz < rhs.sz;
			}
		};
		struct block_compare_address:std::binary_function<block,block,bool>
		{
			bool operator()(const block& lhs,const block& rhs)const{
				return lhs.start < rhs.start;
			}
		};
		static block memory_pool_; //resize exponentially
		typedef std::multiset<block,block_compare
			, /*FSBAllocator<block>*/
			boost::fast_pool_allocator<block,
			boost::default_user_allocator_new_delete,
			boost::details::pool::null_mutex> > FreeBlockSet; // Ordered by block's size
		static FreeBlockSet free_block_set;

		struct block_hash_fun:boost::hash<char*>
		{
			typedef boost::hash<char*> parent;
			std::size_t operator()(block const& val) const
			{
				return parent::operator () (val.start);
			}
		};

		typedef boost::unordered_set<
			block,block_hash_fun,block_start_equal
			,boost::fast_pool_allocator<block,
			boost::default_user_allocator_new_delete,
			boost::details::pool::null_mutex> 
		> AllocatedBlockSet;
		typedef AllocatedBlockSet HashedBlockSet; //hashed by char*

		typedef boost::unordered_set<
			block,block_hash_fun,block_exact_equal
			,boost::fast_pool_allocator<block,
			boost::default_user_allocator_new_delete,
			boost::details::pool::null_mutex> 
		> HashedExactBlockSet;

		static AllocatedBlockSet allocated_block_set;
		HashedBlockSet free_blocks;//help to merge to free blocks

		typedef 
		std::vector<void*,boost::fast_pool_allocator<void*,
			boost::default_user_allocator_new_delete,
			boost::details::pool::null_mutex> > ChunkArray;
			HashedExactBlockSet pool_allocated_chunks[MA_ARCH_BIT - 1];

		
		const static int MAX_POOL_SIZE = power2<MA_ARCH_BIT-2>::value - 1;//512 * 1024 * 1024 - 1;

		typedef boost::unordered_set<void*> MallocatedBlockSet;
		static boost::unordered_set<void*> mallocated_blocks;//handle the situation if memory required exceed max_pool_size
		
		enum SameBlockType {NOT_SAME,SAME_ADDRESS ,SAME_BLOCK};

		template<std::size_t N>
		SameBlockType find_in_pool_allocated_chunks(const block& blk)
		{
			BOOST_STATIC_ASSERT(N < sizeof(pool_allocated_chunks) / sizeof( pool_allocated_chunks[0]));

			//for (std::size_t i = 0;i < MA_ARCH_BIT - 1; ++i)
			//{
				for (HashedExactBlockSet::iterator it = pool_allocated_chunks[N].begin();
					it != pool_allocated_chunks[N].end();++it)
				{
					if (it->start == blk.start && it->sz == blk.sz)
					{
						return SAME_BLOCK;
					}
					else if (it->start == blk.start)
					{
						return SAME_ADDRESS;
					}
				}
			//}
			return NOT_SAME;
		}
		template<std::size_t N>
		static inline void* malloc_impl(std::size_t sz)
		{
			typedef boost::singleton_pool<char[N],sizeof(char[N]),boost::default_user_allocator_new_delete,boost::details::pool::null_mutex> singleton_poolN;
			char* p_head = reinterpret_cast<char*>( singleton_poolN::malloc() );
			if(p_head)
			{
				if(N>sz)
				{
					free_block_set.insert(block(p_head+sz,N-sz));
					free_blocks.insert(block(p_head+sz,N-sz));
				}
				allocated_block_set.insert(block(p_head,sz));

				pool_allocated_chunks[log2<N>::value].insert(block(p_head,N));
			}
			return p_head;
		}

		typedef std::set<block,block_compare_address>  BlockAddressSet;

		template<std::size_t N>
		static inline bool release_unused_impl(BlockAddressSet& /*blocks*/)
		{
			typedef boost::singleton_pool<char[N],sizeof(char[N]),boost::default_user_allocator_new_delete,boost::details::pool::null_mutex> singleton_poolN;


			//for (BlockAddressSet::iterator it = blocks.begin();it != blocks.end();)
			//{
			//	SameBlockType sbt = find_in_pool_allocated_chunks<log2<N>::value>(*it);
			//	if(sbt == SAME_BLOCK)//lucky to find a same block
			//	{
			//		singleton_poolN::free(it->start);

			//		pool_allocated_chunks[log2<N>::value].erase(*it);
			//		blocks.erase(it++);
			//	}
			//	else if (sbt == SAME_ADDRESS) //most case
			//	{
			//		assert(it->sz < N);

			//		BlockAddressSet::iterator next = it;
			//		++next;
			//		while (next != blocks.end())//extending it->sz as far as possible
			//		{
			//			if(it->start+it->sz == next->start && it->sz + next->sz <= N)
			//			{
			//				it->sz = it->sz + next->sz;
			//				blocks.erase(next++);
			//			}
			//			else if (it->sz == N)
			//			{
			//				singleton_poolN::free(it->start);
			//				pool_allocated_chunks[log2<N>::value].erase(*it);
			//				blocks.erase(it++);	
			//				break;
			//			}
			//			else if ( it->start + it->sz != next->start)//cannot continued
			//			{
			//				++it;
			//				break;
			//			}
			//			else ++next;

			//		}
			//	}
			//	else
			//		++it;
			//}


			for (HashedExactBlockSet::iterator it = pool_allocated_chunks[log2<N>::value].begin();
				it != pool_allocated_chunks[log2<N>::value].end();)
			{
				singleton_poolN::free(it->start);
				pool_allocated_chunks[log2<N>::value].erase(it++);
			}
			return singleton_poolN::release_memory();
		}
	}


	bool MAMemoryPool::clean_unused()
	{
		//for (MallocatedBlockSet::iterator it = mallocated_blocks.begin(); it != mallocated_blocks.end(); ++it)
		//{
		//	free(*it);
		//}
		assert(mallocated_blocks.size() == 0);

		//merge all free_blocks
		
		BlockAddressSet
			free_block_by_add(free_block_set.begin(),free_block_set.end());

		free_block_set.clear();
		free_blocks.clear();
	
#define MA_MEMORY_POOL_RELEASE_UNUSED_IMPL( z, N , nothing )\
	ret |= release_unused_impl<power2<N>::value>(free_block_by_add); 

#define MA_MEMORY_POOL_CLEAN_UNUSED(M)\
	BOOST_PP_REPEAT( M, MA_MEMORY_POOL_RELEASE_UNUSED_IMPL , _ )

		bool ret = false;
			MA_MEMORY_POOL_CLEAN_UNUSED( BOOST_PP_SUB( MA_ARCH_BIT , 1 ))
			ret |= release_unused_impl<MAX_POOL_SIZE>(free_block_by_add)
			;

			
			return ret;
#undef MA_MEMORY_POOL_RELEASE_UNUSED_IMPL
#undef MA_MEMORY_POOL_CLEAN_UNUSED
	}


	void* MAMemoryPool::malloc(std::size_t sz)//mark as allocated
	{
		//search in free_block_set
		FreeBlockSet::iterator it = free_block_set.lower_bound(block(0,sz));
		if(it != free_block_set.end())
		{
			std::size_t new_fb_sz = it->sz - sz;
			char* p_head = it->start;
			
			free_blocks.erase(*it);//must find it
			free_block_set.erase(it);
			
			if(new_fb_sz != 0)
			{
				free_block_set.insert(block(p_head+sz,new_fb_sz));
				free_blocks.insert(block(p_head+sz,new_fb_sz));
			}
			allocated_block_set.insert(block(p_head,sz));

			return p_head;
		}

		//if not found, malloc using boost.pool
		//32 case map sz to which pool

#define MA_MEMORY_POOL_DISPATCH_CASE(z, N, data)\
	case power2<N>::value: return malloc_impl<power2<N>::value>(sz);

#define MA_MEMORY_POOL_SWITCH_MALLOC(M)\
	BOOST_PP_REPEAT(M, MA_MEMORY_POOL_DISPATCH_CASE, _)

		assert(std::lower_bound(pool_sizes,pool_sizes+ sizeof(pool_sizes)/sizeof(pool_sizes[0]),sz) != pool_sizes+ sizeof(pool_sizes)/sizeof(pool_sizes[0]));
		switch (*std::lower_bound(pool_sizes,pool_sizes+ sizeof(pool_sizes)/sizeof(pool_sizes[0]),sz))
		{
		MA_MEMORY_POOL_SWITCH_MALLOC( BOOST_PP_SUB (MA_ARCH_BIT , 3 ) )
		default:
			//assert(false);
			//things go ugly
		{
			if(sz < MAX_POOL_SIZE)
			{
				return malloc_impl<MAX_POOL_SIZE>(sz);
			}
			else
				return *(mallocated_blocks.insert(malloc(sz)).first);
		}
			//return 0;
		}

#undef MA_MEMORY_POOL_SWITCH_MALLOC
#undef MA_MEMORY_POOL_DISPATCH_CASE

	}
	void MAMemoryPool::free(void* p)//mark as freed
	{
		char* p_free = reinterpret_cast<char*>(p);
		AllocatedBlockSet::iterator it = allocated_block_set.find(block(p_free,0));
		if(it == allocated_block_set.end())
		{
			//assert(false);
			if(!p) return;
			MallocatedBlockSet::iterator it = mallocated_blocks.find(p);
			if (it!=mallocated_blocks.end())
			{
				mallocated_blocks.erase(it);
				free(*it);
				return;
			}
			assert(false);
			return;
		}
		//merge backward it if possible

		HashedBlockSet::iterator free_it = free_blocks.find(block((char*)p + it->sz,0));//hashed by pointer address
		if (free_it != free_blocks.end())
		{
			//insert merged block
			block back_block(*free_it);

			//erase exist p+it->sz
			free_blocks.erase(free_it);

			free_block_set.insert(block((char*)p, it->sz + back_block.sz));
			free_blocks.insert(block((char*)p, it->sz + back_block.sz));

#ifdef _DEBUG
			bool erased=false;
#endif
			for (FreeBlockSet::iterator fbs_it = free_block_set.lower_bound(back_block); 
				fbs_it != free_block_set.upper_bound(back_block);
				++fbs_it)
			{
				if(fbs_it->start ==  back_block.start)
				{
					//merge
#ifdef _DEBUG
					erased=true;
#endif
					free_block_set.erase(fbs_it);
					break;
				}
			}
			assert(erased);


		}
		else
		{
			free_block_set.insert(block(p_free,it->sz));//insert to free block set
			free_blocks.insert(block(p_free,it->sz));
		}
		allocated_block_set.erase(it); // free from allocated blocks


	}
}
}
