#ifndef MAMEMORYPOOL_TEST_H
#define MAMEMORYPOOL_TEST_H

#include "MABigPool.h"
#include "MAMemoryPool.h"
#include "Timer.h"
using namespace ma;
using namespace core;

//MABigMemoryPool<> default_pool;
MappedMABigMemoryPool default_pool;

int condition_break2(bool cond)
{
	int take_break = 0;
	if(cond)
		take_break = 1;
	return take_break;
}

int condition_break(bool cond)
{
	int take_break = 0;
	if(cond)
		take_break = 1;
	return take_break;
}
//costly computation
unsigned long long fibonacci(std::size_t n)
{
	unsigned long long a0=0;
	unsigned long long a1=1;
	std::size_t i = 0;
	while(i++ < n){
		a1=a0+a1;
		a0=a1;
	}
	return a1;
}

//simulate piratical memory usage as extremly as possible
inline unsigned long long ma_mempool_test_detail(const std::vector<unsigned int>& random_seq)
{
	unsigned long long ret = 0;//avoid compiler optimization
	std::vector<void*> v;
	static const int memory_factor = 512/*1024  * 8*/;
	for(size_t i = 0;i < random_seq.size() ; ++i)
	{
		condition_break2(i == 271 || i == 1941);
		char* p = (char*)default_pool.malloc(random_seq[i] * sizeof(char[memory_factor]));
		//char* p = new char[random_seq[i] * memory_factor];
		v.push_back(p);
		if(p)
			memset(p,0,random_seq[i] * sizeof(char[memory_factor]));

		
	}
	for (size_t i = 0; i < random_seq.size(); ++i)
	{
		//for(size_t j = 0;j < random_seq[i]; ++j)
		//{
			if(!v[i])std::cout<<"NULL pointer!"<<std::endl;
			if(ret < (size_t(-1) >> 1))
				ret += fibonacci(random_seq[i]/* + j*/);
			else
				ret -= ret;
			
		//}
		if(v[i])
			*((int*)v[i]) = (int)ret;
	}
	for (size_t i = 0; i < random_seq.size(); ++i)
	{
		
		if(v[i])
			ret = ret > * ((unsigned long long*) v[i]) ? 0: ret;
		//delete []v[i];
		default_pool.free(v[i]);
	}
	for (size_t i = 0; i < random_seq.size(); ++i)
	{
		for(size_t j = 0;j < random_seq[i]; ++j)
			if(ret < (size_t(-1) >> 1))
				ret += fibonacci(random_seq[i] + j);
			else
				ret -= ret > fibonacci(random_seq[i] + j) ? fibonacci(random_seq[i] + j) : ret;
	}
	return ret;
}
inline void ma_mempool_test(const std::vector<unsigned int>& random_seq,int iterations)
{
	using namespace ma::perf;
	Timer t;
	unsigned long long accum=0; //avoid compiler optimization
	t.start();
	for (int i = 0;i < iterations; ++i)
	{
		condition_break(i == 8);
		std::vector<unsigned int> small_rand(random_seq);
		std::random_shuffle(small_rand.begin(),small_rand.end());
	
		if(accum < (size_t(-1) >> 1))
			accum  += ma_mempool_test_detail(random_seq);
		else 
			accum -= accum > ma_mempool_test_detail(random_seq)? ma_mempool_test_detail(random_seq): accum;

		accum -= accum > ma_mempool_test_detail(small_rand)? accum : 0;

	}
	t.end();
	t.show();
	default_pool.clean_unused();
	std::cerr<<accum<<std::endl;
}
#endif

