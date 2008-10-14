////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008 by luozhiyuan (luozhiyuan@gmail.com)
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author makes no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////


#ifndef OBJECTMEMPOOL_HPP
#define OBJECTMEMPOOL_HPP




#include "Mutex.hpp"





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

		typedef bool (*ReleaseFuncPtrType)();
		
		//default to be boost pool
		template<template<typename T,typename Mtx> class SmallObjSingletonPoolT ,template<typename T,typename Mtx> class BigObjSingletonPoolT,typename Mutex = NullMutex, typename MemoryHandle = void*>
		class ObjectMemoryPool
		{
		public:
			static const size_t small_size = 2048;//less than 1024 is small object

			template<typename T,typename Mtx=Mutex>
			struct SingletonPool: boost::mpl::if_c< (sizeof(T)>small_size), BigObjSingletonPoolT<T,Mtx>,SmallObjSingletonPoolT<T,Mtx> >::type
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
				
#ifdef _DEBUG
				size_t t = sizeof(T);
#endif
				return SingletonPool<T,Mutex>::malloc();
			}

			template<typename T>
			static void freeMemory(MemoryHandle mem,size_type )
			{
#ifdef _DEBUG
				size_t t = sizeof(T);
#endif

				SingletonPool<T,Mutex>::free(mem);
			}

			template<typename T>
			static bool releaseUnused() //not very userful
			{
#ifdef _DEBUG
				size_t t = sizeof(T);
#endif
				return SingletonPool<T,Mutex>::release_memory();
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
		template<template<typename T,typename Mtx> class SmallObjSingletonPoolT ,template<typename T,typename Mtx> class BigObjSingletonPoolT,typename Mutex , typename MemoryHandle >
		std::vector<ReleaseFuncPtrType> ObjectMemoryPool<SmallObjSingletonPoolT, BigObjSingletonPoolT,Mutex,MemoryHandle>::release_funcs_;
	}
}

#include <boost/type_traits.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/singleton_pool.hpp>
#include "FSBSingletonPool.hpp"
namespace ma{
	namespace core{

		template<typename T,class Mutex> 
		struct DefaultFSBSingletonPool:FSBSingletonPool<T,sizeof(T),FSBAllocator<char[sizeof(T)]>,Mutex>
		{};
		template<typename T,class Mutex> 
		struct DefaultFSB2SingletonPool:FSBSingletonPool
			<T,sizeof(T),FSBAllocator2< char[sizeof(T)] >,Mutex >
		{};

		template<typename T,class Mutex> 
		struct BoostSingletonPool:public boost::singleton_pool<T,sizeof(T),boost::default_user_allocator_malloc_free,Mutex>
		{
		};

		typedef ObjectMemoryPool<BoostSingletonPool,BoostSingletonPool,NullMutex> BoostObjMemPool;
		typedef ObjectMemoryPool<DefaultFSBSingletonPool,BoostSingletonPool,NullMutex> FSB_BOOST_ObjMemPool;
		typedef ObjectMemoryPool<DefaultFSB2SingletonPool,BoostSingletonPool,NullMutex> FSB2_BOOST_ObjMemPool;

		//thread safe pools
		typedef ObjectMemoryPool<BoostSingletonPool,BoostSingletonPool,boost::details::pool::default_mutex> BoostObjMemPoolTS;
		typedef ObjectMemoryPool<DefaultFSBSingletonPool,BoostSingletonPool,boost::details::pool::default_mutex> FSB_BOOST_ObjMemPoolTS;
		typedef ObjectMemoryPool<DefaultFSB2SingletonPool,BoostSingletonPool,boost::details::pool::default_mutex> FSB2_BOOST_ObjMemPoolTS;

	}
}



#ifdef _DEBUG

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif

#endif