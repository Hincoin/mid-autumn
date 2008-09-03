#ifndef OBJECTMEMPOOL_H
#define OBJECTMEMPOOL_H


#include <boost/type_traits.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/singleton_pool.hpp>

#include "Mutex.h"




namespace ma
{
	namespace core
	{
		//

		template<typename T,class Mutex=boost::details::pool::default_mutex> 
		struct BoostSingletonPool:public boost::singleton_pool<T,sizeof(T),boost::default_user_allocator_malloc_free,Mutex>
		{};

		//default to be boost pool
		template<template<typename T,typename Mtx> class SingletonPool=BoostSingletonPool,typename Mutex = NullMutex, typename MemoryHandle = void*>
		class ObjectMemoryPool
		{
		private:
			ObjectMemoryPool(const ObjectMemoryPool&);
			ObjectMemoryPool& operator=(const ObjectMemoryPool&);
		private:
			typedef size_t size_type;			
		protected:

		protected:
			ObjectMemoryPool(){}
			~ObjectMemoryPool(){
				//destroy all
			}
		public:

			template<typename T>
			static  MemoryHandle getMemory()
			{
				return SingletonPool<T,Mutex>::malloc();
			}

			template<typename T>
			static  MemoryHandle getArrayMemory(size_type n)// get n * sizeof(T)
			{
				return SingletonPool<T,Mutex>::ordered_malloc(n);
			}

			template<typename T>
			static void freeMemory(MemoryHandle mem,size_type )
			{
				SingletonPool<T,Mutex>::free(mem);
			}

			template<typename T>
			static void releaseUnused() //not very userful
			{
				SingletonPool<T,Mutex>::release_memory();
			}

		};	
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

#define REGISTER_RELEASE_FUN(ClassName)\
	namespace ma{\
namespace core{\
	namespace{\
	}\
}\
}


#endif