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
	unsigned short short_max = std::numeric_limits<unsigned short>::max();
	std::vector<unsigned short> rand_size_seq;
	srand(unsigned int (time(0)));
	for (unsigned short i = 0; i < short_max ; ++i)
	{
		rand_size_seq.push_back((unsigned short (rand()) % 128)+1);
	}
	using namespace ma::perf;
	Timer timer;
	timer.start();
	obj_mempool_test(rand_size_seq);
	timer.end();
	timer.show();

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