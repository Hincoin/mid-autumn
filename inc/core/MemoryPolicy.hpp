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

#ifndef MEMORYPOLICY_HPP
#define MEMORYPOLICY_HPP


#include "MemoryMgr.hpp"

#include "MemoryPool.hpp"
#include "ObjectMemoryPool.hpp"
#include "MAMemoryPool.hpp"
#include "MostDerivedType.hpp"
#include "MAMemPoolSized.hpp"


//test another pool performance but I'm not authorized for the code
//#include "poolmalloc.h"

//Select proper memory policy
//The more time you saved, The more space you lost.
namespace ma
{
	namespace core{

		enum Policies{DEFAULT_NEW_DELETE,MANAGED_BY_SIZE,MANAGED_BY_TYPE,MIXIN_SIZE_TYPE};

		template<Policies p,typename PoolType>struct MemoryPolicySelector;

		struct DEFAULT_NEW_DELETE_POOL_TYPE{};
		template<>
		struct MemoryPolicySelector<DEFAULT_NEW_DELETE,DEFAULT_NEW_DELETE_POOL_TYPE> {
			template<typename T>
			class MemoryPolicy{
			public:
				typedef EmptyType MemoryPoolType;

				static void * operator new(size_t size)
				{
					return ::operator new(size);
					//return pool_malloc(size);
				}
				static void operator delete(void *rawmemory, size_t ){
					return ::operator delete(rawmemory);
					//return pool_free(rawmemory);
				}
				static void *operator new[]( size_t n )
				{ 
					return ::operator new[](n); 
					//return pool_malloc(n);
				}
				static void operator delete[]( void *p, size_t )
				{ 
					return ::operator delete[](p); 
					//return pool_free(p);
				}
			protected:
				MemoryPolicy(){}
				~MemoryPolicy(){}
			};
		};

		template<typename PoolType>
		struct MemoryPolicySelector<MANAGED_BY_SIZE,PoolType> {
			typedef MemoryMgr<PoolType> MemoryPoolType;
			
			template<typename T>
			class MemoryPolicy{	
			public:
				static void * operator new(size_t size)
				{
					return MemoryPoolType::getInstance().getMemory(size);
				}
				static void operator delete(void *rawmemory, size_t ){
					return MemoryPoolType::getInstance().freeMemory(rawmemory);
				}
				static void *operator new[]( size_t n )
				{ return MemoryPoolType::getInstance().getMemory(n); }
				static void operator delete[]( void *p, size_t )
				{ return MemoryPoolType::getInstance().freeMemory(p); }
			protected:
				MemoryPolicy(){}
				~MemoryPolicy(){}
			};
		};

		template<typename PoolType>
		struct MemoryPolicySelector<MANAGED_BY_TYPE,PoolType> {
			typedef MemoryMgr<PoolType> MemoryPoolType;
			
			template<typename T>
			class MemoryPolicy{	
				template<typename ObjectType>
				struct AllocateIndirection{
					static void* malloc()
					{
						typedef typename MostDerivedType<ObjectType>::type AllocType;
						return MemoryPoolType::getInstance().template getMemory<AllocType>(); 	
					}
					static void free(void* p,size_t s)
					{
						typedef typename MostDerivedType<ObjectType>::type AllocType;
						return MemoryPoolType::getInstance().template freeMemory<AllocType> (p,s);
					}
				};
			public:
				static void * operator new(size_t)
				{
					return AllocateIndirection<T>::malloc();
				}
				static void operator delete(void *rawmemory, size_t s){
					return AllocateIndirection<T>::free(rawmemory,s);	
				}
				static void *operator new[]( size_t n )//unpredictable array size
				{
					return malloc(n);
				}
				//{ return MemoryPoolType::getInstance().template getArrayMemory<T>(n/sizeof(typename MostDerivedType<T>::type)); }
				static void operator delete[]( void *p, size_t 
#ifdef _DEBUG
					sz
#endif
					)
				{
					free(p);
				}
				//{ return MemoryPoolType::getInstance().template freeMemory<T>(p,s); }
			protected:
				MemoryPolicy(){}
				~MemoryPolicy(){}
			};

		};
		template<typename PoolType>
			struct MemoryPolicySelector<MIXIN_SIZE_TYPE,PoolType>{
				typedef MemoryMgr<PoolType> MemoryPoolType;

			private:
				template<typename ObjectType>
				struct AllocateIndirection{
					static void* malloc()
					{
						typedef typename MostDerivedType<ObjectType>::type AllocType;
						return MemoryPoolType::getInstance().template getMemory<AllocType>(); 	
					}
					static void free(void* p,size_t )
					{
						typedef typename MostDerivedType<ObjectType>::type AllocType;
						return 
							MemoryPoolType::getInstance().template freeMemory<AllocType>(p);
					}
				};
			public:
				template<typename T>
				class MemoryPolicy{	
				private:
		
				public:
					 
					static void * operator new(size_t)
					{
						return AllocateIndirection<T>::malloc();
					}
					static void operator delete(void *rawmemory, size_t s){
						return AllocateIndirection<T>::free(rawmemory,s);	
					}
					 static  void *operator new[]( size_t n )//unpredictable array size
					{
						return MemoryPoolType::getInstance().getMemory(n);
					}
					//{ return MemoryPoolType::getInstance().template getArrayMemory<T>(n/sizeof(typename MostDerivedType<T>::type)); }
					 static void operator delete[]( void *p, size_t sz)
					{
						return MemoryPoolType::getInstance().freeMemory(p,sz);
					}
					//{ return MemoryPoolType::getInstance().template freeMemory<T>(p,s); }
				protected:
					MemoryPolicy(){}
					~MemoryPolicy(){}
				};
			};


		
		//typedef MemoryPolicySelector<MANAGED_BY_TYPE,BoostObjMemPool> MemoryPolicyType;

		//typedef MemoryPolicySelector<MIXIN_SIZE_TYPE, MABOOSTObjPoolSized/*MADefaultPoolSized*/> MemoryPolicyType;
		//typedef MemoryPolicySelector<DEFAULT_NEW_DELETE,DEFAULT_NEW_DELETE_POOL_TYPE> MemoryPolicyType;

		typedef MemoryPolicySelector<MIXIN_SIZE_TYPE, MABOOSTObjPoolSizedSAOPT> MemoryPolicyType;
		//typedef MemoryPolicySelector<DEFAULT_NEW_DELETE,DEFAULT_NEW_DELETE_POOL_TYPE> MemoryPolicyType;
		

	}
}

#endif