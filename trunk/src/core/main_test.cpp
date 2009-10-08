// $Id:   $

//#define __SSE2__
#include "MAConfig.hpp"
#include "Vector.hpp"

#include <iostream>
#include "simple_test_framework.hpp"

//
//
//#include "MADevice.hpp"
//#include "MAMemoryPool_test.hpp"
//using namespace ma::core;
//#define MEMPOOL_TEST
#define VECTOR_TEST
//#define MOVABLE_TEST
//#define  DUFFS_DEVICE_TEST

//#define DEVICE_TEST

//#define CONVERSTION_TEST
//#define M_ANY_TEST
//#define MA_CRTP_MACRO_TEST
//#define MA_MESH_TEST


#ifdef VECTOR_TEST
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Transform.hpp"
void test();
#endif

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
#include "win32_device_test.hpp"
#else
//...
#include "x11_device_test.hpp"

#endif
#endif // DEVICE_TEST

#ifdef CONVERSTION_TEST

#include "type_conversion_test.hpp"

#endif

#ifdef M_ANY_TEST
#include "ma_any_test.hpp"
#endif

#ifdef MA_CRTP_MACRO_TEST
#include "crtp_interface_macro_test.hpp"
#endif

#ifdef MA_MESH_TEST
#include "Mesh_Test.hpp"
#endif

#ifdef DUFFS_DEVICE_TEST
#include "duffs_device_test.hpp"
#endif

namespace ma_test{}

//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
extern void crtp_test();

int main()
{
//#ifdef TBB_PARALLEL
//	tbb::task_scheduler_init init(2);
//#endif
	run_test();
	return 0;
    using namespace ma_test;
	//crtp_test();
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
	//test();
	vector3f v(1,1,1);
	transform3f t;
	t.setIdentity();
	t.translate(vector3f(1,2,3));
	vector3f v1 = t*v;
	std::cout<<v1.norm()<<std::endl;
	matrix44f tm;
	tm<<0.577f,-0.33f,-0.577f,30.f,
		0,0.666666f,-0.577f,30.f,
		-0.577f,-0.333f,-0.577f,30.f,
		0,0,0,1.f;
	transform3f trans (tm);
	vector3f pos (-0.65f,0.52f,-0.53f);
	vector3f posb = trans.linear()*pos;
	vector3f posc = (trans.linear().inverse().transpose()*pos).normalized();
	std::cout<<posb[0]<<" "<<posb[1]<<" "<< posb[2]<<std::endl;
	std::cout<<posc<<std::endl;

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

#ifdef MA_CRTP_MACRO_TEST
	using namespace ma;
	crtp_macro_test();
#endif

#ifdef DEVICE_TEST
	device_test_func();
#endif // DEVICE_TEST

#ifdef MA_MESH_TEST
	mesh_test();
#endif

#ifdef DUFFS_DEVICE_TEST
duffs_device_test();
#endif
   //wchar_t a=0;
    //using namespace std;
    //swprintf("%c",a);
 //   std::string a;
	return 0;
}
