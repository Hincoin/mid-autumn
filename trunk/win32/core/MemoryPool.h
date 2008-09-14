// $Id:   $

#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H



#include "Mutex.h"

#include <boost/type_traits.hpp>

#include <cassert>
namespace ma
{
namespace core
{
	//

	template<class AssocContainer,class Mutex, typename MemoryHandle = void*>
	class MemoryPool
	{
	protected:
		MemoryPool(const MemoryPool&);
		MemoryPool& operator=(const MemoryPool&);
	private:
		typedef AssocContainer SizeToMemPool;

		
		BOOST_STATIC_ASSERT(boost::is_pointer<typename SizeToMemPool::mapped_type>::value);
		
		typedef typename boost::remove_pointer<typename SizeToMemPool::mapped_type>::type PoolType;

		struct SelfPool:Mutex,AssocContainer
		{
		};
		SelfPool pools_;
	protected:
		typedef typename SizeToMemPool::key_type size_type;
	public:
		static const size_type cached_max_size = 1024;
	protected:
		MemoryPool(size_type sz = cached_max_size){
			for (size_type i = 1;i < sz+1;++i)
			{
				pools_.insert(std::make_pair(i,new PoolType(i)));
			}
		}
		~MemoryPool(){
			//destroy all

			ScopeLock<SelfPool> scp_lck(pools_);

			for (SizeToMemPool::iterator it = pools_.begin();it!=pools_.end();++it)
			{
				delete it->second;
			}
			pools_.clear();
		}
	public:
		
		inline MemoryHandle getMemory(size_type n)
		{
			ScopeLock<SelfPool> scp_lck(pools_);

			SizeToMemPool::iterator it = pools_.find(n);
			if (it == pools_.end())
			{
				it = pools_.insert(std::make_pair(n,new PoolType(n))).first;
			}
			return it->second->malloc();
		}
		inline bool freeMemory(MemoryHandle mem,size_type n)
		{
			ScopeLock<SelfPool> scp_lck(pools_);

			//assert(pools_.find(n) != pools_.end());
			SizeToMemPool::iterator it = pools_.find(n);
			if (it !=pools_.end())
			{
				it->second->free(mem);
				return true;
			}
			return false;
		}
		inline void releaseAllUnused() //give the pooled unused memory back to system
		{
			ScopeLock<SelfPool> scp_lck(pools_);
			bool ret = false;
			for (SizeToMemPool::iterator it = pools_.begin(); it != pools_.end(); ++it)
			{
				ret |= it->second->release_memory();

			}
#ifdef _DEBUG
			if (!ret)
			{
				printf("no memory released !\n");
			}
#endif						
		}

	};	
}
}
//boost memory pool

//#include <boost/pool/detail/mutex.hpp>
#include <boost/pool/pool.hpp>
#include "AssocVector.h"


namespace ma{
	namespace core{
		typedef MemoryPool<AssocVector<size_t,boost::pool<>* >,
			boost::details::pool::default_mutex > BoostAVPoolTS;//multi-thread safe

		typedef MemoryPool<AssocVector<size_t,boost::pool<>* >,
			NullMutex > BoostAVPool;//
	}
}


#endif