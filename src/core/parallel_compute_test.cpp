#include <vector>
#include "parallel_compute.hpp"
#include "simple_test_framework.hpp"

#include "pool.hpp"

using namespace ma;
using namespace std;
struct A_S{
	A_S(int c):a(c){}
	
	MA_DECLARE_POOL_NEW_DELETE_MT(A_S)
		
		
	int a;
};


struct new_test_functor:public parallel_range_processor<new_test_functor>{
	const vector<int>& input_ref;
	vector<A_S*>& output;

	new_test_functor(const vector<int>& i,vector<A_S*>& o):input_ref(i),output(o){}
	const vector<int>& input()const{return input_ref;}
	void run(size_t i)const{
		printf("%d,%d \n",i,get_thread_logic_id());
		assert(i<output.size());
		assert(i<input_ref.size());
		output.at(i) = new A_S(input_ref.at(i));
	}
};

struct delete_test_functor:public parallel_range_processor<delete_test_functor>{
	vector<A_S*>& input_ref;
	delete_test_functor(vector<A_S*>& i):input_ref(i){}
	const vector<A_S*>& input()const{return input_ref;}
	void run(size_t i)const{
		delete input_ref[i];
	} 
};

bool mt_pool_test()
{
#ifdef TBB_PARALLEL
	thread_observer task_observer;
	tbb::task_scheduler_init init;
#endif
	vector<int> input;
	vector<A_S*> output;
	bool result = true;
	const size_t N = 100000;
	for (int i = 0;i < N;i++)
	{
		input.push_back(i);
	}
	output.resize(input.size());
	new_test_functor f(input,output);
	parallel_for::run(f,input.size());
	for (size_t i = 0;i < N; ++i)
	{
		result = result && (output[i]->a == i);
	}
	delete_test_functor df(output);
	parallel_for::run(df,output.size());

	return result ;//&& A_S::release_memory();
}
bool parallel_test()
{
	bool result = mt_pool_test();
	return result;
}


REGISTER_TEST_FUNC(parallel,parallel_test)