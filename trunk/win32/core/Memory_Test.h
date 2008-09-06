#ifndef MEMORY_TEST_H
#define MEMORY_TEST_H

#include "MemoryMgr.h"
#include "MemoryPool.h"
#include "ObjectMemoryPool.h"

#include "CoreObject.h"
#include "Timer.h"
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

MA_REGISTER_RELEASE_FUN(CoreObject,CoreObject<>)
MA_REGISTER_RELEASE_FUN(SonObject,SonObject<>)
MA_REGISTER_RELEASE_FUN(GrandSonObject,GrandSonObject<>)
//
//


template<typename T>
inline void test_size(const std::vector<unsigned int>& random_size_seq)
{
	std::vector<T*> objects;
	for (size_t i = 0;i < random_size_seq.size(); ++i)
	{
		objects.push_back(new T);
	}
	for (size_t i = 0;i< random_size_seq.size(); ++i)
	{
		delete objects[i];
	}
}

template<typename T>
inline void test_size_array(const std::vector<unsigned int>& random_size_seq)
{
	std::vector<T*> objects;
	for (size_t i = 0;i < random_size_seq.size(); ++i)
	{
		objects.push_back(new T[random_size_seq[i]]);
	}
	for (size_t i = 0;i< random_size_seq.size(); ++i)
	{
		delete []objects[i];
	}
}
inline void obj_mempool_test(const std::vector<unsigned int>& random_size_seq, int iterations)
{
	typedef CoreObject<> small_object;
	typedef SonObject<> big_object;
	typedef GrandSonObject<> very_big_object;

	using namespace ma::perf;
	Timer t_global;
	Timer t_local;

	t_global.start();

	t_local.start();
	for(int i = 0;i < iterations;++i)
	test_size<small_object>(random_size_seq);
	t_local.end();
	t_local.show();


	t_local.start();
	for(int i = 0;i < iterations;++i)
	test_size_array<small_object>(random_size_seq);
	t_local.end();
	t_local.show();

	t_local.start();
	for(int i = 0;i < iterations;++i)
	test_size<big_object>(random_size_seq);
	t_local.end();
	t_local.show();

	t_local.start();
	for(int i = 0;i < iterations;++i)
	test_size_array<big_object>(random_size_seq);
	t_local.end();
	t_local.show();

	t_local.start();
	for(int i = 0;i < iterations;++i)
	test_size<very_big_object>(random_size_seq);
	t_local.end();
	t_local.show();

	t_local.start();
	for(int i = 0;i < iterations;++i)
	test_size_array<very_big_object>(random_size_seq);
	t_local.end();
	t_local.show();

	//MemoryPolicyType::MemoryPoolType::getInstance().releaseAllUnused();

	t_global.end();
	t_global.show();
}
#endif