#include "simple_test_framework.hpp"

bool pool_test();


REGISTER_TEST_FUNC(POOL,pool_test)



#include <stdio.h>
#include <stdlib.h>
#include<algorithm>
#include <vector>
#include <map>
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
	return	( rand()%(512));
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
			if (m.back() == 0) m.pop_back();
		}
		else 
		{
			m.push_back(my_pool.realloc(0,MEMORY_SIZE));
			if (m.back() == 0) m.pop_back();

		}
		//assert(checkMemory(m));
	}
}
template<typename PoolT>
void test_pool_free(vector<void*>& m,PoolT& my_pool)
{
	for(size_t i = 0;i < m.size(); ++i)
	{
		if(i == 8)
			int a_break = 0;
		my_pool.realloc(m[i],0);
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
		a[i] = (char)i;
	}
}
};


template <int N>
struct char_size_no_pool{char a[N]; 
char_size_no_pool(){
	for (size_t i = 0; i <  N; ++i)
	{
		a[i] = (char)i;
	}
}
static bool release_memory(){return true;}
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
	//typedef fixed_pool_impl_small<sizeof(T),details::default_user_allocator_malloc_free> pool_t;
	//cout << "" << (int)pool_t::bucket_size_ <<" " << pool_t::sub_page_size_ << " " <<(int)pool_t::sub_page_count_<<endl;
	//cout <<details::PAGE_SIZE<< (details::PAGE_SIZE - 10)/(pool_t::sub_page_size_) <<endl;
	for (unsigned i = 0;i < N;++i)
	{
		v.push_back(new T);
	}
	for (unsigned i = 0;i < N;++i)
	{
		delete v[idx[i]];
	}
	v.clear();
	for (unsigned i = 0;i < N;++i)
		v.push_back(new T);
	T::release_memory();	
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
		a.push_back(new B_TEST[N%128]);
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
#define ENABLE_PERFORMANCE_TEST
#ifdef ENABLE_PERFORMANCE_TEST
	const size_t N = 1024*4; //for big
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
			printf("loop:%d",n*(sizeof(int) + 20));
			while (n--)
			{
				v.push_back(n);
				s.insert(n);
			}
			printf("allocator test done\n");
		}
	}
	catch (...)
	{
		result = false;
	}

	generic_singleton_pool<pool_allocator_tag, details::mutex_t >::release_memory();

	return result;
}

bool mt_singleton_pool_test()
{
	bool result = true;

	result = result && fixed_pool_test();//using different threads

	return result;
}
bool realloc_test(unsigned N)
{
	memory_pool<details::null_mutex> my_pool;
	//unsigned M = N;
	//vector<void*> v ;
	//while (M -- )
	//{
	//	void * p = my_pool.realloc(0,rand()%(512));
	//	unsigned K = 512;
	//	while (K--)
	//	{
	//		int debug_stop = 0;
	//		if(N == 1044577)
	//		{
	//			debug_stop = 0;
	//		}
	//		p = my_pool.realloc(p,rand()%(512));
	//	}
	//	v.push_back(p);
	//}
	//my_pool.release_memory();
	//random_shuffle(v.begin(),v.end());
	//while (!v.empty())
	//{
	//	my_pool.realloc(v.back(),0);
	//	v.pop_back();
	//}

	//return my_pool.release_memory();
	FILE* fptr = fopen("trace.txt","r");
	if (!fptr) return true;
	int ptr_addr_r,ptr_addr,sz;
	map<int,void*> ptr_addr_map;
	int debug_count = 0;

	while (EOF != fscanf(fptr,"%x ,%x, %d",&ptr_addr_r,&ptr_addr,&sz))
	{
		debug_count++;
		my_pool.big_memory_allocator_.enableDebug(debug_count > 1105000);
		if (debug_count == 595398)
		{
			int take_break = 0;
		}
		if (ptr_addr == 0)
		{
			void* p = my_pool.realloc(0,sz);
			ptr_addr_map[ptr_addr_r] = p;
		}
		else
		{
			void* p = my_pool.realloc(ptr_addr_map[ptr_addr],sz);
			if (sz != 0)
				ptr_addr_map[ptr_addr_r] = p;
			else
				ptr_addr_map.erase(ptr_addr_r);
		}
	}
	fclose(fptr);
	printf("%d\n",debug_count);
	return my_pool.release_memory();
}
struct run_test_t{
template<typename S>
	void operator()(S)
	{
	timer t;
	t.start_timer();
	bool fixed_r0 = (fixed_pool_more_test<char_size<S::value> >(32*1024*3));
	printf("size:%d fixed_r0   clock: %ld \n",S::value,t.clocks());
	assert(fixed_r0);
	
	}
};
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/vector_c.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/at.hpp>
template<typename IntSeq>
void test_fixed()
{
	boost::mpl::for_each<IntSeq>(run_test_t());
}
struct to_int{
template<typename N>
	struct apply{
		typedef N type;
	};
typedef to_int type;
};
bool pool_test(){
	printf("-----------------------------------------\n");
	//assert(fixed_pool_test());
	//assert(generic_pool_test());
	//assert(mt_singleton_pool_test());	
	const unsigned N = 32*1024 * 4;
	realloc_test(N);
	namespace mpl= boost::mpl;
	typedef	boost::mpl::vector_c<int,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19> int_list0;
	typedef boost::mpl::vector_c<int,33,24,26,28,29,30,33,34,35,37,39,43,41,44,46,48> int_list1;
	typedef boost::mpl::transform<int_list0,to_int>::type int_list0_t;
	typedef boost::mpl::transform<int_list1,to_int>::type int_list1_t;
	test_fixed<int_list0_t>();
	test_fixed<int_list1_t>();
	timer t;
	t.start_timer();
	bool no_pool_fixed_r0 = (fixed_pool_more_test<char_size_no_pool<1> >(N));
	printf("no_pool_fixed_r0   clock: %ld \n",t.clocks());
	t.start_timer();
	bool no_pool_fixed_r1 = (fixed_pool_more_test<char_size_no_pool<2> >(N));
	printf("no_pool_fixed_r1   clock: %ld \n",t.clocks());
	t.start_timer();
	bool no_pool_fixed_r2 = (fixed_pool_more_test<char_size_no_pool<3> >(N));
	printf("no_pool_fixed_r2   clock: %ld \n",t.clocks());
	t.start_timer();
	bool no_pool_fixed_r3 = (fixed_pool_more_test<char_size_no_pool<4> >(N));
	printf("no_pool_fixed_r3   clock: %ld \n",t.clocks());
 
	bool result = true;
	bool result_fix =  fixed_pool_test();
	bool result_gen =  generic_pool_test();
	bool result_mt = mt_singleton_pool_test();
	result = result_fix && result_gen && result_mt ;
	assert(result);

	return result ;
	printf("-----------------------------------------\n");
}
