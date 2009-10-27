#include "MAConfig.hpp"
#include "Vector.hpp"
#include "Transform.hpp"
#include "../../inc/pbr/Ray.hpp"
#include "Point.hpp"

#include <vector>
using namespace ma;
typedef vector_type<float,3>::type vector_t;
//typedef transform_type<float,3>::type transform_t;
typedef Eigen::Transform<float,3> transform_t;
typedef ray_type<float,3>::type ray_t;
typedef point_type<float,3>::type point_t;
typedef normal_type<float,3>::type normal_t;
struct B{
	B operator-(const B&){}
};
using namespace Eigen;
struct C{
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	char a;
	transform_t b;
};
struct D{
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	char c;
	C x;
};
bool test(){
	//std::vector<transform_t> v;
	//vector_t a,b;
	//vector_t c = a-b;
	//point_t p(1,1,1);
	//vector_t v(1,1,1);
	//normal_t n(v);
	//n.normalize();
	//ray_t r(p,v);
	//transform_t t;
	//t.identity();
	//t.translate(vector_t(1,2,3));
	//point_t p_ = t * p;
	//vector_t v_ = t* v;
	//ray_t r_=t * r;
	//normal_t n_ = t * n;
	//transform_t t0;
	//t0.identity();
	//t0 = t0 * t;
	//C* cc = new C;
	//cc->b.setIdentity();

	//D* dd = new D;
	//dd->x.b.setIdentity();
	//cc->b = dd->x.b;
	//delete dd;
	//delete cc;

	return true;
}
#include "simple_test_framework.hpp"
REGISTER_TEST_FUNC(vector3,test)