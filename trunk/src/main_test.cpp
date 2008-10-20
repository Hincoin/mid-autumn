// $Id:   $

//#define __SSE2__
#include "Vector.hpp"
#include "MADeviceWin32.hpp"


//
//
//#include "MADevice.hpp"
//#include "MAMemoryPool_test.hpp"
//using namespace ma::core;
//#define MEMPOOL_TEST
//#define VECTOR_TEST
//#define MOVABLE_TEST
//#define DEVICE_TEST
//#define CONVERSTION_TEST
#define M_ANY_TEST

#ifdef MEMPOOL_TEST
#include "Timer.hpp"
#include "Memory_Test.hpp"
#include <limits>

#include <cstdlib>

#ifdef max
#undef max
#endif

#endif

#ifdef MOVABLE_TEST
#include "move_test.hpp"
#endif

#ifdef DEVICE_TEST
#ifdef WIN32
#include "MADeviceWin32.hpp"
#else
//...
#endif
#endif // DEVICE_TEST

#ifdef CONVERSTION_TEST

#include "type_conversion_test.hpp"

#endif

#ifdef M_ANY_TEST
#include "ma_any_test.hpp"
#endif

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
	using namespace ma;
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
	std::cout<< (m22 )<<std::endl;
	std::cout<<m22.cwise().abs();

	//BOOST_STATIC_ASSERT(sizeof(f2) == 2 * sizeof(float));
#endif

#ifdef MOVABLE_TEST
	test_move();
#endif
#ifdef DEVICE_TEST
	//test device
#endif	

#ifdef CONVERSTION_TEST
	promote_test();
#endif
#ifdef M_ANY_TEST
	many_test();
#endif
	return 0;
}
