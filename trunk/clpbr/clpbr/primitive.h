#ifndef _PRIMITIVE_H_
#define _PRIMITIVE_H_

#include "shape.h"
#include "material.h"
typedef struct  
{
	differential_geometry_t dg;
	unsigned primitive_idx;
	//todo
	float ray_epsilon;
}_intersection_t;

#ifndef CL_KERNEL
#include "make_aligned_type.h"
typedef make_aligned_type<_intersection_t,sizeof(float)*4>::type intersection_t;
#else
typedef _intersection_t intersection_t;
#endif

typedef struct  
{
	shape_info_t shape_info;
	material_info_t material_info;
}primitive_info_t;

#endif