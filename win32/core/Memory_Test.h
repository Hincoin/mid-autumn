#ifndef MEMORY_TEST_H
#define MEMORY_TEST_H

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
	long long chunk_mem[1024*sizeof(Parent)];
};

//MA_REGISTER_RELEASE_FUN(CoreObject,CoreObject<>)
//MA_REGISTER_RELEASE_FUN(SonObject,SonObject<>)
//MA_REGISTER_RELEASE_FUN(GrandSonObject,GrandSonObject<>)
//
//
inline void obj_mempool_test(const std::vector<unsigned short>& random_size_seq)
{
	typedef CoreObject<> small_object;
	typedef SonObject<> big_object;
	typedef GrandSonObject<> very_big_object;
	for (size_t i = 0;i < random_size_seq.size(); ++i)
	{
		big_object* the_son = new big_object;

		very_big_object* the_grand = new very_big_object;

		big_object* the_son_array = new big_object[random_size_seq[i]];

		very_big_object* the_grand_array = new very_big_object[random_size_seq[i]];

		delete the_son;
		delete the_grand;
		delete []the_son_array;
		delete []the_grand_array;	
	}
	MemoryPolicyType::MemoryPoolType::getInstance().releaseAllUnused();
}
#endif