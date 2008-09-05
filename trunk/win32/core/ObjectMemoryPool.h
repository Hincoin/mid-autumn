#ifndef OBJECTMEMPOOL_H
#define OBJECTMEMPOOL_H


#include <boost/type_traits.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/singleton_pool.hpp>

#include "Mutex.h"


#include "MostDerivedType.h"


#ifdef _DEBUG

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4189)
#endif

#endif

namespace ma
{
	namespace core
	{
		//

		

		template<typename T,class Mutex=boost::details::pool::default_mutex> 
		struct BoostSingletonPool:public boost::singleton_pool<T,sizeof(T),boost::default_user_allocator_malloc_free,Mutex>
		{
		};

		typedef bool (*ReleaseFuncPtrType)();
		
		//default to be boost pool
		template<template<typename T,typename Mtx> class SingletonPoolT=BoostSingletonPool,typename Mutex = NullMutex, typename MemoryHandle = void*>
		class ObjectMemoryPool
		{
		public:
			template<typename T,typename Mtx=Mutex>
			struct SingletonPool:SingletonPoolT<T,Mtx>
			{};
		private:
			ObjectMemoryPool(const ObjectMemoryPool&);
			ObjectMemoryPool& operator=(const ObjectMemoryPool&);
		protected:
			typedef size_t size_type;			
		
			

			typedef std::vector<ReleaseFuncPtrType> FuncPtrs; 
			static  FuncPtrs release_funcs_;
		protected:
			ObjectMemoryPool(){}
			~ObjectMemoryPool(){
				//destroy all
			}
		public:

			template<typename T>
			static  MemoryHandle getMemory()
			{
				typedef typename MostDerivedType<T>::type AllocType; 
#ifdef _DEBUG
				size_t t = sizeof(AllocType);
#endif
				return SingletonPool<AllocType,Mutex>::malloc();
			}

			template<typename T>
			static  MemoryHandle getArrayMemory(size_type n)// get n * sizeof(T)
			{
				typedef typename MostDerivedType<T>::type AllocType; 
#ifdef _DEBUG
				size_t t = sizeof(AllocType);
#endif

				return SingletonPool<AllocType,Mutex>::ordered_malloc(n);
			}

			template<typename T>
			static void freeMemory(MemoryHandle mem,size_type )
			{
				typedef typename MostDerivedType<T>::type AllocType; 
#ifdef _DEBUG
				size_t t = sizeof(AllocType);
#endif

				SingletonPool<AllocType,Mutex>::free(mem);
			}

			template<typename T>
			static void releaseUnused() //not very userful
			{
				typedef typename MostDerivedType<T>::type AllocType; 
#ifdef _DEBUG
				size_t t = sizeof(AllocType);
#endif
				SingletonPool<AllocType,Mutex>::release_memory();
			}

			static void releaseAllUnused() //give the pooled unused memory back to system
			{
				for(FuncPtrs::iterator it = release_funcs_.begin();it != release_funcs_.end();++it)
					(*(*it))();
			}

			static void registerReleaseFunc(ReleaseFuncPtrType fptr)
			{
				release_funcs_.push_back(fptr);
			}
		};	
		template<template<typename T,typename Mtx> class SingletonPoolT ,typename Mutex , typename MemoryHandle >
		std::vector<ReleaseFuncPtrType> ObjectMemoryPool<SingletonPoolT,Mutex,MemoryHandle>::release_funcs_;
	}
}

#include "FSBSingletonPool.h"
namespace ma{
	namespace core{

		template<typename T,class Mutex=boost::details::pool::default_mutex> 
		struct DefaultFSBSingletonPoolTS:FSBSingletonPool<T,sizeof(T)>
		{};

		template<typename T,class Mutex=NullMutex> 
		struct DefaultFSBSingletonPool:FSBSingletonPool<T,sizeof(T)>
		{};

		typedef ObjectMemoryPool<> BoostObjMemPool;
		typedef ObjectMemoryPool<DefaultFSBSingletonPool,NullMutex> FSBObjMemPool;
	}
}



#ifdef _DEBUG

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif

#endif