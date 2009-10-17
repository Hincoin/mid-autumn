// $Id:   $

//#define __SSE2__
#include "MAConfig.hpp"
#include "simple_test_framework.hpp"


int main()
{
//#ifdef TBB_PARALLEL
//	tbb::task_scheduler_init init(2);
//#endif
	run_test();
	return 0;
}

//#include <Eigen/Core>
//#include <Eigen/LU>
//#include <Eigen/QR>
//#include <Eigen/Geometry>
//#include <Eigen/Array>
//typedef Eigen::Transform<float,3> transform_t;
//using namespace Eigen;
//struct C{
//	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
//
//		char a;
//	transform_t b;
//};
//struct D{
//	//EIGEN_MAKE_ALIGNED_OPERATOR_NEW
//	
//	C x;char c;
//};
//int main(){
//
//	C* cc = new C;
//	cc->b.setIdentity();
//	char* m = new char[13];
//	D* dd = new /*(m+1)*/ D;
//	dd->x.b.setIdentity();
//	cc->b = dd->x.b;
//	delete dd;
//	delete cc;
//
//	return 0;
//}
