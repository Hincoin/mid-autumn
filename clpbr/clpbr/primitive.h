#ifndef _PRIMITIVE_H_
#define _PRIMITIVE_H_

#include "shape.h"
#include "material.h"
typedef struct  
{
	differential_geometry_t dg;
	unsigned primitive_idx;
	//todo
}intersection_t;

typedef struct  
{
	shape_info_t shape_info;
	material_info_t material_info;
}primitive_info_t;

#endif