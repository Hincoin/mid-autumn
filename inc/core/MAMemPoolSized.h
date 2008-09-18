#ifndef MADEFAULTMEMPOOL_H
#define MADEFAULTMEMPOOL_H


#include "MemoryPool.h"
#include "Mutex.h"

#ifdef __GNUC__
#define MA_NEW_DELETE_ARRAY_OPTIMIAZATION
#endif // _DEBUG

#if defined(MA_NEW_DELETE_ARRAY_OPTIMIAZATION) && defined(_MSC_VER)
#pragma message("Don't define MA_NEW_DELETE_ARRAY_OPTIMIAZATION when using a Microsoft compiler to prevent memory leaks.")
#pragma message("now calling '#undef MA_NEW_DELETE_ARRAY_OPTIMIAZATION'")
#undef MA_NEW_DELETE_ARRAY_OPTIMIAZATION
#endif

#include <boost/unordered_map.hpp>
namespace ma
{
namespace core
{
	struct NullPool{
		typedef std::size_t size_type;
		inline void* getMemory(size_type  )
		{
			return 0;
		}
		inline bool freeMemory(void*  , size_type  )
		{
			return false;
		}
		inline bool releaseAllUnused()
		{
			return false;
		}

	};
	template<typename Pool>
	struct SmallArrayPool:protected Pool{
		typedef std::size_t size_type;
#if defined(MA_NEW_DELETE_ARRAY_OPTIMIAZATION)
		inline void* getMemory(size_type n)
		{
			return Pool::getMemory(n);
		}
		inline bool freeMemory(void* mem, size_type s)
		{
			return Pool::freeMemory(mem,s);
		}

#else
		//typedef std::map<char*,size_t> Allocated;
		typedef boost::unordered_map<char*,size_t> Allocated;
		Allocated allocated_;
		inline void* getMemory(size_type n)
		{
			return (((allocated_.insert(std::make_pair((char*)( Pool::getMemory(n)),n))).first)->first);
		}
		inline bool freeMemory(void* mem, size_type)
		{
			Allocated::iterator it = allocated_.find((char*)mem);
			if(it!=allocated_.end())
			{
				Pool::freeMemory(it->first,it->second);
				allocated_.erase(it);
				return true;
			}
			return false;
		}

#endif
		inline void releaseAllUnused()
		{
			Pool::releaseAllUnused();
		}
	};
	//2 aspects describe a pool: small allocator,big allocator
	template<typename FixedPool,typename VariablePool,typename Mutex,typename MemoryHandle = void*>
	struct MADefaultMemPoolSized{
	private:
		typedef std::size_t size_type;
		struct SelfPool:Mutex,protected FixedPool, protected VariablePool
		{
			typedef FixedPool FixedPool_;
			typedef VariablePool VariablePool_;

			typedef typename MADefaultMemPoolSized::size_type size_type;
			//free by object type
			template<typename T>
			inline MemoryHandle getMemory()
			{
				return FixedPool_::getMemory(sizeof(T));
			}
			template<typename T>
			inline void freeMemory(MemoryHandle mem)
			{
				FixedPool_::freeMemory(mem,sizeof(T));
			}
			inline MemoryHandle getMemory(size_type n)
			{
				return VariablePool_::getMemory(n);
			}
			inline void freeMemory(MemoryHandle mem, size_type s)
			{
				VariablePool_::freeMemory(mem,s);
			}
			inline void releaseAllUnused() //give the pooled unused memory back to system
			{
				FixedPool_::releaseAllUnused();
				VariablePool_::releaseAllUnused();
			}
		};
		SelfPool pools_;
	public:

		template<typename T>
		inline MemoryHandle getMemory()
		{
			ScopeLock<SelfPool> scp_lck(pools_);
			return pools_.getMemory<T>();
		}
		template<typename T>
		inline void freeMemory(MemoryHandle mem)
		{
			ScopeLock<SelfPool> scp_lck(pools_);
			pools_.freeMemory<T>(mem);
		}
		inline MemoryHandle getMemory(size_type n)
		{
			ScopeLock<SelfPool> scp_lck(pools_);
			return pools_.getMemory(n);
		}
		inline void freeMemory(MemoryHandle mem,size_type n)
		{
			ScopeLock<SelfPool> scp_lck(pools_);
			pools_.freeMemory(mem,n);
		}
		inline void releaseAllUnused() //give the pooled unused memory back to system
		{
			ScopeLock<SelfPool> scp_lck(pools_);
			pools_.releaseAllUnused();
		}

	};

	//2 aspects describe a pool: small allocator,big allocator
	template<typename FixedPool,typename VariablePool,typename SmallVariablePool,typename Mutex,typename MemoryHandle = void*>
	struct MADefaultObjectPoolSized:Mutex,protected FixedPool,protected VariablePool,SmallVariablePool{
	private:
		typedef std::size_t size_type;
		typedef FixedPool FixedPool_;
		typedef VariablePool VariablePool_;
		typedef SmallVariablePool SmallVariablePool_;
		
		typedef MADefaultObjectPoolSized SelfPool; 
	public:
		static const size_type small_array_size = 512;

		template<typename T>
		inline MemoryHandle getMemory()
		{
			ScopeLock<SelfPool> scp_lck(*this);
			return FixedPool_::template getMemory<T>();
		}
		template<typename T>
		inline void freeMemory(MemoryHandle mem)
		{
			ScopeLock<SelfPool> scp_lck(*this);
			FixedPool_::template freeMemory<T>(mem,1);
		}
		inline MemoryHandle getMemory(size_type n)
		{
			ScopeLock<SelfPool> scp_lck(*this);
			MemoryHandle p=0; 
			if(n < small_array_size && (p = SmallVariablePool_::getMemory(n)))
			{
				return p;
			}
			p = VariablePool_::getMemory(n);
			
			if (!p)
			{
				releaseAllUnused();
				return VariablePool_::getMemory(n);
			}
			return p;

			
		}
		inline void freeMemory(MemoryHandle mem,size_type n)
		{
			ScopeLock<SelfPool> scp_lck(*this);
#if defined(MA_NEW_DELETE_ARRAY_OPTIMIAZATION)
			if(n < small_array_size)
				SmallVariablePool_::freeMemory(mem,n);
#else
			if(!SmallVariablePool_::freeMemory(mem,n))
				VariablePool_::freeMemory(mem,n);
#endif
			
		}
		inline void releaseAllUnused() //give the pooled unused memory back to system
		{
			ScopeLock<SelfPool> scp_lck(*this);
			
			SmallVariablePool_::releaseAllUnused();
			FixedPool_::releaseAllUnused();
			VariablePool_::releaseAllUnused();
		}

		static void registerReleaseFunc(ReleaseFuncPtrType fptr)
		{
			FixedPool_::registerReleaseFunc(fptr);
		}
		template<typename T>
		static bool releaseUnused() 
		{
			return FixedPool_::template releaseUnused<T>();
		}
	};

}
}
#include "MemoryPool.h"
#include "MAMemoryPool.h"
#include "ObjectMemoryPool.h"
namespace ma
{
	namespace core
	{
	typedef MADefaultMemPoolSized<BoostAVPool,MappedMAMemoryBigPool,NullMutex> 
		MADefaultPoolSized;
	typedef MADefaultObjectPoolSized<FSB_BOOST_ObjMemPool,MappedMAMemoryBigPool,NullPool,NullMutex>
		MAFSBBOOSTPoolSized;
	typedef MADefaultObjectPoolSized<BoostObjMemPool,MappedMAMemoryBigPool,NullPool,NullMutex>
		MABOOSTObjPoolSized;
	typedef MADefaultObjectPoolSized<BoostObjMemPool,MappedMAMemoryBigPool,SmallArrayPool<BoostAVPool>,NullMutex>
		MABOOSTObjPoolSizedSAOPT;//small array optimized
	}
}

#endif
