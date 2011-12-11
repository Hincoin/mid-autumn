#ifndef _SHAPE_H_
#define _SHAPE_H_

#include "differential_geometry.h"


typedef struct  
{
	unsigned shape_type;
	unsigned memory_start;
}shape_info_t;


INLINE void UniformSampleSphere(const float u1, const float u2, vector3f_t *v) {
	const float zz = 1.f - 2.f * u1;
	const float r = sqrt(max(0.f, (1.f - zz * zz)));
	const float phi = 2.f * FLOAT_PI * u2;
	const float xx = r * cos(phi);
	const float yy = r * sin(phi);
	vinit(*v, xx, yy, zz);
}
enum ShapeType{TRIANGLE_VERTEX8=1,TRIANGLE_VERTEX16=2, TRIANGLE_VERTEX32=4};
#endif