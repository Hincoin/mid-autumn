#ifndef MAMEMORYPOOL_TEST_H
#define MAMEMORYPOOL_TEST_H

#include "MAMemoryPool.h"
#include "Timer.h"
using namespace ma;
using namespace core;

inline void ma_mempool_test_detail(const std::vector<unsigned int>& random_seq)
{
	std::vector<void*> v;
	for(size_t i = 0;i < random_seq.size(); ++i)
		v.push_back(MAMemoryPool::malloc(random_seq[i] * sizeof(char[1024 * 8])));
	for (size_t i = 0; i < random_seq.size(); ++i)
	{
		MAMemoryPool::free(v[i]);
	}
}
inline void ma_mempool_test(const std::vector<unsigned int>& random_seq,int iterations)
{
	using namespace ma::perf;
	Timer t;
	t.start();
	for (size_t i = 0;i < iterations; ++i)
	{
		ma_mempool_test_detail(random_seq);
	}
	t.end();
	t.show();
	MAMemoryPool::clean_unused();
}
#endif

