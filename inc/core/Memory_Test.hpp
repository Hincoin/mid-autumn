#ifndef MEMORY_TEST_HPP
#define MEMORY_TEST_HPP

#include "MemoryMgr.hpp"
#include "MemoryPool.hpp"
#include "ObjectMemoryPool.hpp"

#include "CoreObject.hpp"
#include "Timer.hpp"
using namespace ma;
using namespace core;


#include <iostream>
//#include "poolmalloc.h"

template<typename Derived = EmptyType>
class SonObject:public PooledCoreObject<SonObject<Derived> >
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
//
MA_REGISTER_RELEASE_FUN(PooledCoreObject,PooledCoreObject<>)
MA_REGISTER_RELEASE_FUN(SonObject,SonObject<>)
MA_REGISTER_RELEASE_FUN(GrandSonObject,GrandSonObject<>)
//
//


template<typename T>
inline int test_size(const std::vector<unsigned int>& random_size_seq)
{
	int ret =0;
	std::vector<T*> objects;
	for (size_t i = 0;i < random_size_seq.size(); ++i)
	{
		objects.push_back(new T  );
		objects.back()->c = (char)random_size_seq[i];
	}
	for (size_t i = 0;i< random_size_seq.size(); ++i)
	{
		ret += objects[i]->c;
		delete objects[i];
		//pool_free(objects[i]);
	}
	return ret;
}

template<typename T>
inline int test_size_array(const std::vector<unsigned int>& random_size_seq)
{
	int ret=0;
	std::vector<T*> objects;
	for (size_t i = 0;i < random_size_seq.size(); ++i)
	{
		objects.push_back(new T[random_size_seq[i]] );
		objects.back()[0].c = (char)random_size_seq[i];
	}
	for (size_t i = 0;i< random_size_seq.size(); ++i)
	{
		ret += objects[i][0].c;
		delete []objects[i];
	}
	return ret;
}
inline void obj_mempool_test(const std::vector<unsigned int>& random_size_seq, int iterations)
{
	typedef PooledCoreObject<> small_object;
	typedef SonObject<> big_object;
	typedef GrandSonObject<> very_big_object;

	using namespace ma::perf;
	Timer t_global;
	Timer t_local;
	
	int accum = 0;
	t_global.start();
	
	std::cerr<<"test small_object , size per object "<<sizeof(small_object)<<std::endl;

	t_local.start();
	for(int i = 0;i < iterations;++i)
		accum += accum > test_size<small_object>(random_size_seq)? 1:accum;
	t_local.end();
	t_local.show();
	


	std::cerr<<"test small_object array , size per object "<<sizeof(small_object)<<std::endl;
	t_local.start();
	for(int i = 0;i < iterations;++i)
		accum+= accum > test_size_array<small_object>(random_size_seq)?0:1;
	t_local.end();
	t_local.show();

	

	std::cerr<<"test big_object, size per object "<<sizeof(big_object)<<std::endl;
	t_local.start();
	for(int i = 0;i < iterations;++i)
		accum+= accum > test_size<big_object>(random_size_seq)?1:0;
	t_local.end();
	t_local.show();

	std::cerr<<"test big_object array, size per object "<<sizeof(big_object)<<std::endl;
	t_local.start();
	for(int i = 0;i < iterations;++i)
		accum+= accum> test_size_array<big_object>(random_size_seq)?0:1;
	t_local.end();
	t_local.show();


	std::cerr<<"test very_big_object, size per object "<<sizeof(very_big_object)<<std::endl;
	t_local.start();
	for(int i = 0;i < iterations;++i)
		accum += accum> test_size<very_big_object>(random_size_seq)? 1:0;
	t_local.end();
	t_local.show();

	std::cerr<<"test very_big_object array, size per object "<<sizeof(very_big_object)<<std::endl;
	t_local.start();
	for(int i = 0;i < iterations;++i)
		accum += accum > test_size_array<very_big_object>(random_size_seq)?0:1;
	t_local.end();
	t_local.show();

	std::cerr<<"release unused objects which shouldn't count:"<<std::endl;
	t_local.start();
	t_local.end();
	t_local.show();

	t_global.end();
	std::cerr<<"total time: \n";
	t_global.show();

	std::cerr<<"accum result:"<<accum<<std::endl;
}
#endif