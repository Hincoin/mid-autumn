// $Id:   $

#include "MemoryMgr.h"
#include "MemoryPool.h"
#include "ObjectMemoryPool.h"

#include "CoreObject.h"

using namespace ma;
using namespace core;


#include <iostream>

template<typename Derived = EmptyType>
class SonObject:public CoreObject<SonObject<Derived> >
{
public:
	typedef Derived DerivedType;
	
	std::vector<SonObject> s_array;
	char chunk_mem[1024];
};

template<typename Derived =EmptyType>
class GrandSonObject:public SonObject<GrandSonObject<Derived> >
{
public:
	typedef Derived DerivedType;

	typedef SonObject<GrandSonObject<Derived> > Parent;
	long long chunk_mem[sizeof(Parent)];
};

static bool MA_RELEASE_FUNCTION_sonobj()									
{																				
	typedef MostDerivedType<EmptyType >::type AllocType;				
	typedef ::ma::core::MemoryPolicyType::MemoryPoolType MemPool;										
	return MemPool::template SingletonPool<AllocType>::release_memory();		
}
#undef MA_HAS_MEMFUN_DEFAULT_PARAM_COUNT
#define MA_HAS_MEMFUN_DEFAULT_PARAM_COUNT 1

//MA_REGISTER_RELEASE_FUN(SonObject,SonObject<>)
//MA_REGISTER_RELEASE_FUN(GrandSonObject)
//MA

namespace ma{																		
namespace core{																		
//namespace{																			
//HAS_MEMFUN(registerReleaseFunc)		

#define M 0

	template<int,typename T,typename FuncPtr>										
	struct has_memfun_registerReleaseFunc;//{static const bool value = false;};									
	template<typename T, typename RetType  BOOST_PP_COMMA_IF(M) BOOST_PP_ENUM_PARAMS(M, class T)>		
	struct has_memfun_registerReleaseFunc<0,T, RetType (*) ( BOOST_PP_ENUM_PARAMS(M,T) ) >		
	{	
		typedef RetType (*FunctionPtr) ( BOOST_PP_ENUM_PARAMS(M,T));
		
	typedef char true_type;	
	struct false_type		
	{						
	true_type dummy[2];	
	};						
	
	template<typename U>	
	static true_type has_member(U*,RetType (U::*dummy)( BOOST_PP_ENUM_PARAMS(M,T) )=&U::registerReleaseFunc);	
	static false_type has_member(void*);	
	
	static const bool value=
		sizeof(has_memfun_registerReleaseFunc<T, RetType (*) ( BOOST_PP_ENUM_PARAMS(M,T) )>::has_member((T*)0))
		==sizeof(true_type);	
	};

template<bool>																		
struct MA_STATIC_OBJECT_TYPE_SonObject{};	

	typedef bool (*registeredFunPtr)();										
	typedef void (*FunPtr)(registeredFunPtr);			
	
	
	template<>																			
struct MA_STATIC_OBJECT_TYPE_SonObject											
	<																				
	true																				
	>																				
{																					
	typedef MemoryPolicyType::MemoryPoolType MemPool;								
	MA_RELEASE_FUNCTION(SonObject,SonObject<>)													
	MA_STATIC_OBJECT_TYPE_SonObject(){											
	MemPool::registerReleaseFunc(&MA_RELEASE_FUNCTION_SonObject);				
	}																				
};																					
	static MA_STATIC_OBJECT_TYPE_SonObject<
		has_memfun_registerReleaseFunc<0,MemoryPolicyType::MemoryPoolType,FunPtr>::value
	> 
	MA_STATIC_OBJECT_TYPE_SonObject_object;	
//}
}
}
int main()
{

	typedef CoreObject<> small_object;
	typedef SonObject<> big_object;
	typedef GrandSonObject<> very_big_object;

	FSBObjMemPool::template getMemory<small_object >();

	big_object* the_son = new big_object;

	very_big_object* the_grand = new very_big_object;

	big_object* the_son_array = new big_object[100];

	very_big_object* the_grand_array = new very_big_object[1000];

	delete the_son;
	delete the_grand;
	delete []the_son_array;
	delete []the_grand_array;

	return 0;
}