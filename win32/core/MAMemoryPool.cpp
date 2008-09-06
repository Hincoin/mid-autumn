#include "MAMemoryPool.h"

#include "FSBAllocator.h"

#include <set>
#include <boost/unordered_set.hpp>
#include <boost/pool/singleton_pool.hpp>
#include <boost/pool/pool_alloc.hpp>

#include <boost/preprocessor.hpp>

namespace ma
{
namespace core
{
	namespace{
		//this meta-function is only for this Memory pool
	template<size_t N>
	struct power2{
		static const unsigned int value = N < 31 ? (1 << N) :((unsigned int(-1)) >> 1);
	};

#define MA_GENERATE_POWER2(z, N , data)\
	power2<N>::value BOOST_PP_COMMA_IF( BOOST_PP_SUB(data, N) )

#define MA_POOL_SIZES\
	BOOST_PP_REPEAT(32, MA_GENERATE_POWER2 , 31)

		static const unsigned int pool_sizes[]= {
		MA_POOL_SIZES
		/*	1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,
		2*8192,4*8192,8*8192,16*8192,32*8192,64*8192,128*8192,256*8192,512*8192,1024*8192,2048*8192,4096*8192,8192*8192,
		8192*8192*2,8192*8192*4,8192*8192*8,8192*8192*16,8192*8192*32,8192*8192*64*/
		};//32 bit


		struct block{
			block():start(0),sz(0){}
			block(char* st,size_t siz):start(st),sz(siz){}
			char* start;
			size_t sz;
		};

		bool operator == (const block&lhs, const block& rhs)
		{
			return lhs.start == rhs.start;
		}
		struct block_compare:std::binary_function<block,block,bool>
		{
			bool operator()(const block& lhs,const block& rhs)const{
				return lhs.sz < rhs.sz;
			}
		};
		static block memory_pool_; //resize exponentially
		typedef std::multiset<block,block_compare, boost::fast_pool_allocator<block,
			boost::default_user_allocator_new_delete,
			boost::details::pool::null_mutex> > FreeBlockSet; // FSBAllocator
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
			block,block_hash_fun,std::equal_to<block>,boost::fast_pool_allocator<block,
			boost::default_user_allocator_new_delete,
			boost::details::pool::null_mutex> 
		> AllocatedBlockSet;
		static AllocatedBlockSet allocated_block_set;


		const static int MAX_POOL_SIZE = 512 * 1024 * 1024 - 1;

		typedef boost::unordered_set<void*> MallocatedBlockSet;
		static boost::unordered_set<void*> mallocated_blocks;//handle the situation if memory required exceed max_pool_size

		template<size_t N>
		static inline void* malloc_impl(size_t sz)
		{
			typedef boost::singleton_pool<char[N],sizeof(char[N]),boost::default_user_allocator_new_delete,boost::details::pool::null_mutex> singleton_poolN;
			char* p_head = reinterpret_cast<char*>( singleton_poolN::malloc() );
			free_block_set.insert(block(p_head+sz,N-sz));
			allocated_block_set.insert(block(p_head,sz));
			return p_head;
		}
		template<size_t N>
		static inline bool release_unused_impl()
		{
			typedef boost::singleton_pool<char[N],sizeof(char[N]),boost::default_user_allocator_new_delete,boost::details::pool::null_mutex> singleton_poolN;
			return singleton_poolN::release_memory();
		}
	}
#define MA_RELEASE_UNUSED_IMPL( z, N , nothing )\
	release_unused_impl<power2<N>::value>() && 

#define MA_CLEAN_UNUSED(M)\
	BOOST_PP_REPEAT( M, MA_RELEASE_UNUSED_IMPL , _ )

	bool MAMemoryPool::clean_unused()
	{
		for (MallocatedBlockSet::iterator it = mallocated_blocks.begin(); it != mallocated_blocks.end(); ++it)
		{
			free(*it);
		}
		return 
			MA_CLEAN_UNUSED(31)
			release_unused_impl<MAX_POOL_SIZE>()
			;
	}
#undef MA_RELEASE_UNUSED_IMPL
#undef MA_CLEAN_UNUSED

	void* MAMemoryPool::malloc(size_t sz)//mark as allocated
	{
		//search in free_block_set
		FreeBlockSet::iterator it = free_block_set.lower_bound(block(0,sz));
		if(it != free_block_set.end())
		{
			size_t new_fb_sz = it->sz - sz;
			char* p_head = it->start;
			free_block_set.erase(it);
			if(new_fb_sz != 0)
				free_block_set.insert(block(p_head+sz,new_fb_sz));
			allocated_block_set.insert(block(p_head,sz));

			return p_head;
		}

		//if not found, malloc using boost.pool
		//32 case map sz to which pool

#define MA_DISPATCH_CASE(z, N, data)\
	case power2<N>::value: return malloc_impl<power2<N>::value>(sz);

#define MA_SWITCH_MALLOC(M)\
	BOOST_PP_REPEAT(M, MA_DISPATCH_CASE, _)

		assert(std::lower_bound(pool_sizes,pool_sizes+ sizeof(pool_sizes)/sizeof(pool_sizes[0]),sz) != pool_sizes+ sizeof(pool_sizes)/sizeof(pool_sizes[0]));
		switch (*std::lower_bound(pool_sizes,pool_sizes+ sizeof(pool_sizes)/sizeof(pool_sizes[0]),sz))
		{
		MA_SWITCH_MALLOC(29)
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

#undef MA_SWITCH_MALLOC
#undef MA_DISPATCH_CASE

	}
	void MAMemoryPool::free(void* p)//mark as freed
	{
		char* p_free = reinterpret_cast<char*>(p);
		AllocatedBlockSet::iterator it = allocated_block_set.find(block(p_free,0));
		if(it == allocated_block_set.end())
		{
			//assert(false);
			MallocatedBlockSet::iterator it = mallocated_blocks.find(p);
			if (it!=mallocated_blocks.end())
			{
				free(*it);
				return;
			}
			assert(false);
			return;
		}
		free_block_set.insert(block(p_free,it->sz));//insert to free block set
		allocated_block_set.erase(it); // free from allocated blocks
	}
}
}