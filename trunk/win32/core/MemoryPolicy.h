#ifndef MEMORYPOLICY_H
#define MEMORYPOLICY_H


#include "MemoryMgr.h"

#include "MemoryPool.h"
#include "ObjectMemoryPool.h"

#include <new>
namespace ma
{
	namespace core{

		enum Policies{DEFAULT_NEW_DELETE,MANAGED_BY_SIZE,MANAGED_BY_TYPE/*,MIXIN_SIZE_TYPE*/};

		template<Policies p>struct MemoryPolicySelector;

		template<>
		struct MemoryPolicySelector<DEFAULT_NEW_DELETE> {
			template<typename T>
			class MemoryPolicy{
			public:
				typedef EmptyType MemoryPoolType;

				static void * operator new(size_t size)
				{
					return ::operator new(size);
				}
				static void operator delete(void *rawmemory, size_t size){
					return ::operator delete(rawmemory);
				}
				static void *operator new[]( size_t n )
				{ return ::operator new[](n); }
				static void operator delete[]( void *p, size_t )
				{ return ::operator delete[](p); }
			protected:
				MemoryPolicy(){}
				~MemoryPolicy(){}
			};
		};

		template<>
		struct MemoryPolicySelector<MANAGED_BY_SIZE> {
			typedef MemoryMgr<BoostAVPool> MemoryPoolType;
			
			template<typename T>
			class MemoryPolicy{	
			public:
				static void * operator new(size_t size)
				{
					return MemoryPoolType::getInstance().getMemory(size);
				}
				static void operator delete(void *rawmemory, size_t){
					return MemoryPoolType::getInstance().freeMemory(rawmemory);
				}
				static void *operator new[]( size_t n )
				{ return MemoryPoolType::getInstance().getMemory(size); }
				static void operator delete[]( void *p, size_t )
				{ return MemoryPoolType::getInstance().freeMemory(p); }
			protected:
				MemoryPolicy(){}
				~MemoryPolicy(){}
			};
		};

		template<>
		struct MemoryPolicySelector<MANAGED_BY_TYPE> {
			typedef MemoryMgr<BoostObjMemPool> MemoryPoolType;
			
			template<typename T>
			class MemoryPolicy{	
			public:
				static void * operator new(size_t)
				{
					return MemoryPoolType::getInstance().template getMemory<T>();
				}
				static void operator delete(void *rawmemory, size_t s){
					return MemoryPoolType::getInstance().template freeMemory<T> (rawmemory,s);
				}
				static void *operator new[]( size_t n )
				{ return MemoryPoolType::getInstance().template getArrayMemory<T>(n/sizeof(typename MostDerivedType<T>::type)); }
				static void operator delete[]( void *p, size_t s)
				{ return MemoryPoolType::getInstance().template freeMemory<T>(p,s); }
			protected:
				MemoryPolicy(){}
				~MemoryPolicy(){}
			};
		};

		typedef MemoryPolicySelector<MANAGED_BY_TYPE> MemoryPolicyType;

	}
}

#endif