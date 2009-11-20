#include "simple_test_framework.hpp"

bool pool_test();


REGISTER_TEST_FUNC(POOL,pool_test)



#include <stdio.h>
#include <stdlib.h>
#include<algorithm>
#include <vector>
#include <set>
//#define SET_POOL
#ifdef SET_POOL
#include "memory_pool_set.hpp"
#else
#include "pool.hpp"
#endif

#include "pool.hpp"

//#include "hpha.h"
void test_compare();

struct single_thread{};
using namespace ma;
using namespace ma::core;
using namespace std;
typedef big_memory_pool<details::null_mutex> pool_t;

#define REALLOCATE_TEST

BOOST_STATIC_ASSERT(sizeof(MemBlock) == (sizeof(size_t)+sizeof(void*)) );
#include <vector>
#include <ctime>
#include <iostream>
unsigned big_memory_size()
{
	return	( rand()%(1024*1024)+256);
}
unsigned small_memory_size()
{
	return	( rand()%(512)+128);
}
#define MEMORY_SIZE (rand()%2 ? small_memory_size():big_memory_size())

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



template<typename PoolT>
void test_pool_alloc(vector<void*>& m,size_t N,PoolT& my_pool)
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
template<typename PoolT>
void test_pool_free(vector<void*>& m,PoolT& my_pool)
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
bool test_my_pool(unsigned N,unsigned M)
{
	clock_t t;

	srand(0);
	size_t alloc_insert_count = 0;
	memory_pool<details::null_mutex> my_pool;
	t = clock();
	while(M--)
	{
		vector<void* > m;
		size_t tmp = my_pool.big_memory_allocator_.debug_insert_count;
		test_pool_alloc(m,N,my_pool);
		alloc_insert_count += (my_pool.big_memory_allocator_.debug_insert_count - tmp);
		test_pool_free(m,my_pool);
	}my_pool.release_memory();
	printf("pool: %ld \n",clock()-t);
	printf("pool malloc size and found/find count: %d , %d, %d/%d found rate:%.6f\n",my_pool.big_memory_allocator_.debug_allocated_size,
		my_pool.big_memory_allocator_.debug_total_size,my_pool.big_memory_allocator_.debug_found_count,my_pool.big_memory_allocator_.debug_find_count,
		float(my_pool.big_memory_allocator_.debug_found_count)/my_pool.big_memory_allocator_.debug_find_count);
	printf("pool malloc insert count: %d\n",my_pool.big_memory_allocator_.debug_insert_count);
	printf("my pool malloc insert count in alloc and free :%d, %d \n",alloc_insert_count,my_pool.big_memory_allocator_.debug_insert_count-alloc_insert_count);

	return true;
}

//void test_fixed_pool(unsigned N,unsigned M)
//{
//	clock_t t;
//	fixed_pool<> pool(N);
//	srand(0);
//	size_t alloc_insert_count = 0;
//	t = clock();
//	while(M--)
//	{
//		vector<void*> m;
//	}
//	printf("pool: %ld \n",clock()-t);
//}


int xxxxmain()
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

#ifndef _DEBUG
	//test_hpha(N,M);
	//::Sleep(5000);
#endif
	//test_my_pool(N,M);
	//::Sleep(2000);
	//test_no_pool(N,M);
	//::Sleep(2000);



	return 0;
}

struct A_TEST{
	MA_DECLARE_POOL_NEW_DELETE(A_TEST)

	char t[10];
	virtual ~A_TEST(){}
};
struct B_TEST:A_TEST{
	MA_DECLARE_POOL_NEW_DELETE_MT(B_TEST)

	char b[20];
};
template <int N>
struct char_size{char a[N];MA_DECLARE_POOL_NEW_DELETE(char_size);
char_size(){
	for (size_t i = 0; i <  N; ++i)
	{
		a[i] = i;
	}
}
};
template<typename T>
bool fixed_pool_more_test(unsigned N )
{
	std::vector<unsigned> idx;
	for (unsigned i = 0;i < N; ++i)
	{
		idx.push_back(i);
	}
	std::random_shuffle(idx.begin(),idx.end());
	std::vector<T*> v;
	using namespace ma::core;
	typedef fixed_pool_impl_small<sizeof(T),details::default_user_allocator_malloc_free> pool_t;
	cout << "" << (int)pool_t::bucket_size_ <<" " << pool_t::sub_page_size_ << " " <<(int)pool_t::sub_page_count_<<endl;
	cout <<details::PAGE_SIZE<< (details::PAGE_SIZE - 10)/(pool_t::sub_page_size_) <<endl;
	for (unsigned i = 0;i < N;++i)
		v.push_back(new T);
	for (unsigned i = 0;i < N;++i)
	{
		delete v[idx[i]];
	}
	return T::release_memory();	
}
bool fixed_pool_test()
{
	//
	const int N = 10000;
	//boost::pool<> p(sizeof(B_TEST));
	//std::vector<void*> boost_pool;
	//for(int i = 0;i < N; i ++)
	//	boost_pool.push_back(p.malloc());
	//for(int i = 0;i < N; i++)
	//	p.free(boost_pool[i]);
	//bool r = p.purge_memory();
	bool result = true;
	
	std::vector<A_TEST*> a;
	int t = 0;
	for (int i = 0;i < N;i++)
	{
		a.push_back(new B_TEST);
	}
	for (int i = 0;i < N;i++)
	{
		delete a[i];
		if (B_TEST::release_memory())t++;
	}
	a.clear();
	bool b = t > 0 && !ma::core::fixed_singleton_pool<B_TEST,sizeof(B_TEST),ma::core::details::mutex_t>::release_memory();
	t = 0;
	for (int i = 0;i < N;i++)
	{
		a.push_back(new B_TEST[3]);
	}
	for (int i = 0;i < N;i++)
	{
		delete [] ((B_TEST*) a[i]);
		if(B_TEST::release_memory()) 
			t++;
	}
	result =  t > 0;
	return result && b;
}
bool generic_pool_test()
{
	bool result = true;
#ifdef ENABLE_PERFORMANCE_TEST
	const size_t N = 1024*32; //for big
	//const size_t N = 1024 * 1024 *8;//for small
	const size_t M = 256;// 
#else
	const size_t N = 64;
	const size_t M = 4;
#endif


	//test_compare();
	//compute total size we should malloc
	try
	{
		srand(0);
		result = test_my_pool(N,M);

		{
			std::vector<int,pool_allocator<int,details::mutex_t> > v;
			std::set<int,std::less<int>,pool_allocator<int,details::mutex_t> > s;
			int n = int(N*N);
			while (n--)
			{
				v.push_back(n);
				s.insert(n);
			}
		}
	}
	catch (...)
	{
		result = false;
	}


	return result;
}

bool mt_singleton_pool_test()
{
	bool result = true;

	result = result && fixed_pool_test();//using different threads

	return result;
}

bool pool_test(){
	printf("-----------------------------------------\n");
	//assert(fixed_pool_test());
	//assert(generic_pool_test());
	//assert(mt_singleton_pool_test());	
	assert(fixed_pool_more_test<char_size<1> >(32*1024 * 32));
	bool result = true;
	bool result_fix =  fixed_pool_test();
	bool result_gen =  generic_pool_test();
	bool result_mt = mt_singleton_pool_test();
	result = result_fix && result_gen && result_mt;
	assert(result);

	return result ;
	printf("-----------------------------------------\n");
}
