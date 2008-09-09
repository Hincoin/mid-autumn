#ifndef MAMEMORYPOOL_TEST_H
#define MAMEMORYPOOL_TEST_H

#include "MABigObjectPool.h"
#include "MAMemoryPool.h"
#include "Timer.h"
using namespace ma;
using namespace core;

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

//simulate piratical memory usage as I can
inline unsigned long long ma_mempool_test_detail(const std::vector<unsigned int>& random_seq)
{
	unsigned long long ret = 0;//avoid compiler optimization
	std::vector<void*> v;
	static const int memory_factor = 1024 * 8;
	for(size_t i = 0;i < random_seq.size() ; ++i)
	{
		//v.push_back(MAMemoryPool::malloc(random_seq[i] * sizeof(char[memory_factor])));
		v.push_back(new char[random_seq[i] * memory_factor]);
		
		//v.push_back(MABigObjectPool<>::malloc(random_seq[i] * sizeof(char[memory_factor])));
		if(v.back())
			memset(v.back(),0,random_seq[i]);
		
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
		//MAMemoryPool::free(v[i]);
		if(v[i])
			ret = ret > * ((unsigned long long*) v[i]) ? 0: ret;
		delete []v[i];
		//MABigObjectPool<>::free(v[i]);
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
		if(accum < (size_t(-1) >> 1))
			accum  += ma_mempool_test_detail(random_seq);
		else 
			accum -= accum > ma_mempool_test_detail(random_seq)? ma_mempool_test_detail(random_seq): accum;
		if (accum < iterations)
		{
			std::vector<unsigned int> small_rand(random_seq);
			std::random_shuffle(small_rand.begin(),small_rand.end());
			accum = accum < ma_mempool_test_detail(small_rand)? accum : iterations;
		}
	}
	//MABigObjectPool<>::clean_unused();
	t.end();
	t.show();
	std::cerr<<accum<<std::endl;
}
#endif

