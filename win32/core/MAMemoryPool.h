#ifndef MAMEMORYPOOL_H
#define MAMEMORYPOOL_H

#include <boost/pool/singleton_pool.hpp>
#include "Mutex.h"
//for big object allocation, tested under 32bit arch
namespace ma
{
namespace core
{

	template<typename MPool,typename Mutex = NullMutex>
	struct MAMemPool{
		struct SelfPool:Mutex,MPool{};

		typedef std::size_t size_type;
		SelfPool pool_;
	public:
		void * malloc(size_type s)
		{
			boost::details::pool::guard<Mutex> g(pool_);
			return pool_.malloc(s);
		}
		void free(void * const ptr)
		{
			boost::details::pool::guard<Mutex> g(pool_);
			pool_.free(ptr);
		}
		bool release_memory()
		{
			boost::details::pool::guard<Mutex> g(pool_);
			return pool_.clean_unused();
		}
	};

	template<typename MPool,typename Mutex>
	struct MAMemPoolSingleton{
		struct SelfPool:Mutex,MPool{};
		typedef boost::details::pool::singleton_default<SelfPool> Singleton;

		typedef std::size_t size_type;
	public:
		static void * malloc(size_type s)
		{
			SelfPool & p = Singleton::instance();
			boost::details::pool::guard<Mutex> g(p);
			return p.malloc(s);
		}
		static void free(void * const ptr)
		{
			SelfPool & p = Singleton::instance();
			boost::details::pool::guard<Mutex> g(p);
			p.free(ptr);
		}
		static bool release_memory()
		{
			SelfPool & p = Singleton::instance();
			boost::details::pool::guard<Mutex> g(p);
			return p.clean_unused();
		}
	};


}
}

#include "MABigPool.h"

namespace ma{
	namespace core{
		typedef MAMemPoolSingleton<MABigMemoryPool<>,NullMutex> MAMemoryBigPoolSingleton;
		typedef MAMemPoolSingleton<MappedMABigMemoryPool, NullMutex> MappedMAMemoryBigPoolSingleton;

		typedef MAMemPoolSingleton<MABigMemoryPool<>,
			boost::details::pool::default_mutex> MAMemoryBigPoolTSSingleton;
		typedef MAMemPoolSingleton<MappedMABigMemoryPool,
			boost::details::pool::default_mutex> MappedMAMemoryBigPoolTSSingleton;

		template<typename PoolImpl>
		struct MAMemPoolAdapter:PoolImpl{
		private:
			typedef std::size_t size_type;
		public:
			 void* getMemory(size_type n)
			{
				return PoolImpl::malloc(n);
			}
			  void freeMemory(void* mem,size_type)
			{
				return PoolImpl::free(mem);
			}
			 void releaseAllUnused() //give the pooled unused memory back to system
			{
				PoolImpl::release_memory();
			}
			template<typename T>
			void* getMemory(){
				return PoolImpl::malloc(sizeof(T));
			}
		};

		typedef MAMemPoolAdapter<
			MAMemPool<
			MABigMemoryPool<ma_detail::default_mem_allocator_new_delete ,ma_detail::BigMemPoolDefaultTag
			> > >  MAMemoryBigPool;
		typedef MAMemPoolAdapter<MAMemPool<MappedMABigMemoryPool> > MappedMAMemoryBigPool;

		typedef MAMemPoolAdapter<
			MAMemPool<
			MABigMemoryPool<ma_detail::default_mem_allocator_new_delete ,ma_detail::BigMemPoolDefaultTag
		> ,boost::details::pool::default_mutex> >  MAMemoryBigPoolTS;
		typedef MAMemPoolAdapter<MAMemPool<MappedMABigMemoryPool,boost::details::pool::default_mutex> > MappedMAMemoryBigPoolTS;



		typedef MAMemPoolAdapter<MAMemoryBigPoolSingleton>  MAMemoryBigPoolSingle;
		typedef MAMemPoolAdapter<MappedMAMemoryBigPoolSingleton> MappedMAMemoryBigPoolSingle;

		typedef MAMemPoolAdapter<MAMemoryBigPoolTSSingleton>  MAMemoryBigPoolTSSingle ;
		typedef MAMemPoolAdapter<MappedMAMemoryBigPoolTSSingleton> MappedMAMemoryBigPoolTSSingle ;
	}
}
#endif