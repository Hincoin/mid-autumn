#include "Vector.hpp"
#include "Transform.hpp"
#include "../inc/pbr/Ray.hpp"
#include "Point.hpp"

using namespace ma;
typedef vector_type<float,3>::type vector_t;
typedef transform_type<float,3>::type transform_t;
typedef ray_type<float,3>::type ray_t;
typedef point_type<float,3>::type point_t;
typedef normal_type<float,3>::type normal_t;
struct B{
	B operator-(const B&){}
};
using namespace Eigen;
struct C:B{};
void test(){
	vector_t a,b;
	vector_t c = a-b;
	point_t p(1,1,1);
	vector_t v(1,1,1);
	normal_t n(v);
	n.normalize();
	ray_t r(p,v);
	transform_t t;
	t.identity();
	t.translate(vector_t(1,2,3));
	point_t p_ = t * p;
	vector_t v_ = t* v;
	ray_t r_=t * r;
	normal_t n_ = t * n;
	transform_t t0;
	t0.identity();
	t0 = t0 * t;
}