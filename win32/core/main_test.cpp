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

MA_REGISTER_RELEASE_FUN(SonObject,SonObject<>)
MA_REGISTER_RELEASE_FUN(GrandSonObject,GrandSonObject<>)



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