#ifndef OBJECTMEMPOOL_H
#define OBJECTMEMPOOL_H


#include <boost/type_traits.hpp>

#include "Mutex.h"

namespace ma
{
	namespace core
	{
		//



		template<class AssocContainer,class Mutex, typename MemoryHandle = void*>
		class ObjectMemoryPool
		{
		private:
			ObjectMemoryPool(const ObjectMemoryPool&);
			ObjectMemoryPool& operator=(const ObjectMemoryPool&);
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
		protected:
			ObjectMemoryPool(){}
			~ObjectMemoryPool(){
				//destroy all

				ScopeLock<SelfPool> scp_lck(pools_);

				for (SizeToMemPool::iterator it = pools_.begin();it!=pools_.end();++it)
				{
					delete it->second;
				}
				pools_.clear();
			}
		public:

			template<typename T>
			inline MemoryHandle getMemory(size_type n)
			{
				ScopeLock<SelfPool> scp_lck(pools_);

				SizeToMemPool::iterator it = pools_.find(n);
				if (it == pools_.end())
				{
					it = pools_.insert(std::make_pair(n,new PoolType(n))).first;
				}
				return it->second->ordered_malloc();
			}

			template<typename T>
			inline void freeMemory(MemoryHandle mem,size_type n)
			{
				ScopeLock<SelfPool> scp_lck(pools_);

				assert(pools_.find(n) != pools_.end());
				SizeToMemPool::iterator it = pools_.find(n);
				if (it !=pools_.end())
				{
					it->second->free(mem);
				}
			}

			template<typename T>
			inline void releaseAllUnused()
			{
				ScopeLock<SelfPool> scp_lck(pools_);
				for (SizeToMemPool::iterator it = pools_.begin(); it != pools_.end(); ++it)
				{
					it->second->release_memory();
				}
			}

		};	
	}
}

#endif