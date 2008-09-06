// $Id:   $

#include "Timer.h"
#include "Memory_Test.h"
#include <limits>

#include <cstdlib>

#ifdef max
#undef max
#endif

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


	obj_mempool_test(rand_size_seq);


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