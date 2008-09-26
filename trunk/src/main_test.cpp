// $Id:   $
#include "Vector.h"
#include "Timer.h"
#include "Memory_Test.h"
#include <limits>

#include <cstdlib>

#ifdef max
#undef max
#endif

#include "MAMemoryPool_test.h"
//#define MEMPOOL_TEST
#define VECTOR_TEST

int main()
{
#ifdef MEMPOOL_TEST
	const static unsigned int MAX_MEM = 0x7fffffff;
	unsigned int MAX_ARRAY_SIZE = 128;
	unsigned int MAX_OBJECT_SIZE = 8*1024;
	unsigned int short_max = MAX_MEM/(MAX_ARRAY_SIZE * MAX_OBJECT_SIZE);
	std::vector<unsigned int> rand_size_seq;
	srand(0);
	for (unsigned int i = 0; i < short_max ; ++i)
	{
	  rand_size_seq.push_back(((unsigned int) (rand()) % MAX_ARRAY_SIZE)+1);
	}
	const size_t Iterations = 1024;


	obj_mempool_test(rand_size_seq,Iterations);

#endif
#ifdef VECTOR_TEST
	vector2i i2;
	vector2f f2(1,1);
	//vector2f f22;
	//f2 = vector2f(i2);
	//vector3f f3;
	vector4f f4(2,2,2,2),f4_(1,1,1,1);
	vector4f r = (f4-f4_);
	matrix22f m22=matrix22f::Random();
	matrix22f m222 = m22.inverse();
	//m22.setIdentity();
	;
	std::cout<< (m22 * m222)<<std::endl;
	std::cout<<m222;

	//BOOST_STATIC_ASSERT(sizeof(f2) == 2 * sizeof(float));
#endif


	return 0;
}
