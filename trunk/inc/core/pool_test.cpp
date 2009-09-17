#include <stdlib.h>
#include<algorithm>
//#define SET_POOL
#ifdef SET_POOL
#include "memory_pool_set.hpp"
#else
#include "pool.hpp"
#endif

#include "pool.hpp"

#include <Windows.h>
#include "hpha.h"
void test_compare();

struct single_thread{};
using namespace ma;
using namespace ma::core;
using namespace std;
typedef big_memory_pool pool_t;

#define REALLOCATE_TEST


#include <vector>
#include <ctime>


HPHAllocator hpha_allocator;
memory_pool my_pool;
unsigned big_memory_size()
{
	return	( rand()%(1024*1024)+256);
}
unsigned small_memory_size()
{
	return	( rand()%(512)+128);
}
#define MEMORY_SIZE small_memory_size()

bool checkMemory(const vector<void*>& m)
{
	vector<void*> cp(m);
	std::sort(cp.begin(),cp.end());
	for (;!cp.empty();)
	{
		void* m = cp.back();
		cp.pop_back();
		if (std::binary_search(cp.begin(),cp.end(),m))
		{
			return false;
		}
	}
	return true;
}
void test_hpha_alloc(vector<void*>& m,size_t N)
{
	for (size_t i = 0;i < N;i++)
	{
		unsigned sz = MEMORY_SIZE;
		if(sz > MEMORY_SIZE && ! m.empty())
		{
#ifdef REALLOCATE_TEST
			m.back() = hpha_allocator.realloc(m.back(),MEMORY_SIZE);
#endif
		}
		else
			m.push_back(hpha_allocator.alloc(MEMORY_SIZE));
	}
}
void test_hpha_free(vector<void*>& m)
{
	for (size_t i = 0;i < m.size(); ++i)
	{
		hpha_allocator.free(m[i]);
	}
}
void test_pool_alloc(vector<void*>& m,size_t N)
{
	for(size_t i = 0 ;i < N;i ++)
	{
		if (i == 226)
		{
			int a_break = 0;
		}
		unsigned sz = MEMORY_SIZE;
		if(sz > MEMORY_SIZE && ! m.empty())
		{
#ifdef REALLOCATE_TEST
			m.back() = my_pool.realloc(m.back(),MEMORY_SIZE);
#endif
			//my_pool.free(m.back());m.pop_back();
		}
		else 
			m.push_back(my_pool.alloc(MEMORY_SIZE));
		assert(checkMemory(m));
	}
}
void test_pool_free(vector<void*>& m)
{
	for(size_t i = 0;i < m.size(); ++i)
	{
		if(i == 8)
			int a_break = 0;
		my_pool.free(m[i]);
	}
}

void test_comp_alloc(vector<void*>& m,size_t N)
{
	for (size_t i = 0;i < N; ++i)
	{
		unsigned sz = MEMORY_SIZE;
		if(sz > MEMORY_SIZE && ! m.empty())
		{
#ifdef REALLOCATE_TEST
			m.back() = realloc(m.back(),MEMORY_SIZE);
#endif
		}
		else m.push_back(::malloc(MEMORY_SIZE));
	}
}
void test_comp_free(vector<void*>& m)
{
	for(size_t i = 0;i < m.size();++i)
		::free(m[i]);
}


void test_hpha(unsigned N,unsigned M)
{
	clock_t t;

	size_t alloc_insert_count = 0;
	srand(0);
	t = clock();
	while (M--)
	{
		vector<void*> m;
		size_t tmp = hpha_allocator.debug_insert_count;
		test_hpha_alloc(m,N);
		alloc_insert_count += (hpha_allocator.debug_insert_count - tmp);
		test_hpha_free(m);
	}
	printf("hpha pool malloc size and found/find count: %d, %d,%d/%d  found rate: %.6f \n",hpha_allocator.debug_total_allocated,hpha_allocator.debug_alloc_size,hpha_allocator.debug_found_count,hpha_allocator.debug_find_count,float(hpha_allocator.debug_found_count)/hpha_allocator.debug_find_count);
	hpha_allocator.purge();printf("hpha pool: %ld \n",clock()-t);
	printf("hpha pool malloc insert count : %d \n",hpha_allocator.debug_insert_count);
	printf("hpha pool malloc insert count in alloc and free : %d , %d \n",alloc_insert_count,hpha_allocator.debug_insert_count-alloc_insert_count);

}
void test_no_pool(unsigned N,unsigned M)
{
	clock_t t;

	srand(0);
	t = clock();
	while(M--)
	{
		vector<void*> m;
		test_comp_alloc(m,N);
		test_comp_free(m);
	}

	printf("no pool: %ld \n",clock() - t);

}
void test_my_pool(unsigned N,unsigned M)
{
	clock_t t;

	srand(0);
	size_t alloc_insert_count = 0;
	t = clock();
	while(M--)
	{
		vector<void* > m;
		size_t tmp = my_pool.big_memory_allocator_.debug_insert_count;
		test_pool_alloc(m,N);
		alloc_insert_count += (my_pool.big_memory_allocator_.debug_insert_count - tmp);
		test_pool_free(m);
	}my_pool.release_memory();
	printf("pool: %ld \n",clock()-t);
	printf("pool malloc size and found/find count: %d , %d, %d/%d found rate:%.6f\n",my_pool.big_memory_allocator_.debug_allocated_size,
		my_pool.big_memory_allocator_.debug_total_size,my_pool.big_memory_allocator_.debug_found_count,my_pool.big_memory_allocator_.debug_find_count,
		float(my_pool.big_memory_allocator_.debug_found_count)/my_pool.big_memory_allocator_.debug_find_count);
	printf("pool malloc insert count: %d\n",my_pool.big_memory_allocator_.debug_insert_count);
	printf("my pool malloc insert count in alloc and free :%d, %d \n",alloc_insert_count,my_pool.big_memory_allocator_.debug_insert_count-alloc_insert_count);
}

void test_fixed_pool(unsigned N,unsigned M)
{
	clock_t t;
	fixed_pool<> pool(N);
	srand(0);
	size_t alloc_insert_count = 0;
	t = clock();
	while(M--)
	{
		vector<void*> m;
	}
	printf("pool: %ld \n",clock()-t);
}


int main()
{

#ifndef _DEBUG
	const size_t N = 1024*64; //for big
	//const size_t N = 1024 * 1024 *8;//for small
	const size_t M = 256;// 
#else
	const size_t N = 512;
	const size_t M = 4;
#endif


	//test_compare();
	//compute total size we should malloc
	srand(0);
	unsigned total_mem_size = 0;
	for (size_t i = 0 ;i < N; ++i)
	{
		total_mem_size += MEMORY_SIZE;
	}
	printf("total memory size required: %ld \n",total_mem_size);

#ifndef _DEBUG
	//test_hpha(N,M);
	//::Sleep(5000);
#endif
	//test_my_pool(N,M);
	//::Sleep(2000);
	//test_no_pool(N,M);
	//::Sleep(2000);


	std::vector<int,pool_allocator<int,details::mutex_t> > v;
	int n = N;
	while (n--)
	{
		v.push_back(n);
	}
	return 0;
}