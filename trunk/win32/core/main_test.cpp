// $Id:   $

#include "Timer.h"
#include "Memory_Test.h"
#include <limits>

#include <cstdlib>

#ifdef max
#undef max
#endif


#include "MAMemoryPool_test.h"

int main()
{
	const static unsigned int MAX_MEM = 0x7fffffff;
	unsigned int MAX_ARRAY_SIZE = 128;
	unsigned int MAX_OBJECT_SIZE = 8*1024;
	unsigned int short_max = MAX_MEM/(MAX_ARRAY_SIZE * MAX_OBJECT_SIZE);
	std::vector<unsigned int> rand_size_seq;
	srand(0);
	for (unsigned int i = 0; i < short_max ; ++i)
	{
		rand_size_seq.push_back((unsigned int (rand()) % MAX_ARRAY_SIZE)+1);
	}
	const size_t Iterations = 1024;


	obj_mempool_test(rand_size_seq,Iterations);

	const int N = 8* 1024 * 1024 - 1;//536870912;
	//typedef boost::singleton_pool<char[N],sizeof(char[N]),boost::default_user_allocator_new_delete,boost::details::pool::null_mutex> singleton_poolN;
	//singleton_poolN::malloc();

	//boost::pool<> big_pool(N); 
	typedef boost::singleton_pool<char[N],sizeof(char[N])> sing_pool;

	//void* m = sing_pool::malloc();
	//sing_pool::free(m);
	sing_pool::release_memory();
	//char* m = new char[N];//(char*)malloc(N * sizeof(char));
	//delete []m;//free(m);
	//ma_mempool_test(rand_size_seq,1024);

//	using namespace ma::perf;
//	Timer timer;
//	
//	
//	//FSBAllocator<SIZE_N<8553496> > a;
//	const size_t s = 512;
//	const size_t count = 1024 * 1024;
//	FSBAllocator<char[s] > a;
//
//timer.start();
//for(size_t i = 0;i < count;++i)
//	a.allocate(1);
//	timer.end();
//	timer.show();
//	timer.start();
//	char* c = (char*)malloc(s * count * sizeof(char));
//	free(c);
//	timer.end();
//	timer.show();
//
//	//FSBAllocator<char[128* sizeof(GrandSonObject<>)]> a;
//	std::cout<<sizeof(GrandSonObject<> )<<std::endl;
	return 0;
}