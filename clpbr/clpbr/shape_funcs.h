#ifndef _SHAPE_FUNC_H_
#define _SHAPE_FUNC_H_

#include "shape.h"
#include "quadratic_math.h"


#include "shape.h"
#include "mc.h"

#include "cl_scene.h"
#include "triangle_mesh_ocl.h"

INLINE float shape_area(shape_info_t shape_info,cl_scene_info_t scene_info)
{
	switch(shape_info.shape_type)
	{
	case TRIANGLE_VERTEX8:
		{
			triangle_t triangle;
			unsigned memory_start = shape_info.memory_start;
			unsigned mesh_memory_start = as_uint(scene_info.shape_data[memory_start]);
			unsigned triangle_index = as_uint(scene_info.shape_data[memory_start + 1]);
			load_triangle_vertex8(scene_info.shape_data,mesh_memory_start,triangle_index,&triangle);
			return triangle_area(&triangle);
		}
	default:
		return 0;
	}
}

INLINE void shape_sample_on_shape(shape_info_t* shape_info,cl_scene_info_t scene_info,float u0,float u1,normal3f_t *ns,point3f_t *pr)
{
	if(shape_info->shape_type == TRIANGLE_VERTEX8)
	{
		triangle_t triangle;
		unsigned memory_start = shape_info->memory_start;
		unsigned mesh_memory_start = as_uint(scene_info.shape_data[memory_start]);
		unsigned triangle_index = as_uint(scene_info.shape_data[memory_start + 1]);
		load_triangle_vertex8(scene_info.shape_data,mesh_memory_start,triangle_index,&triangle);
		triangle_sample(&triangle,u0,u1,ns,pr);
	}
}
INLINE void shape_sample(shape_info_t* shape_info,cl_scene_info_t scene_info,const point3f_t* p,float u0,float u1,normal3f_t *ns,point3f_t *pr)
{
	if(shape_info->shape_type == TRIANGLE_VERTEX8)
	{
		triangle_t triangle;
		unsigned memory_start = shape_info->memory_start;
		unsigned mesh_memory_start = as_uint(scene_info.shape_data[memory_start]);
		unsigned triangle_index = as_uint(scene_info.shape_data[memory_start + 1]);
		load_triangle_vertex8(scene_info.shape_data,mesh_memory_start,triangle_index,&triangle);
		triangle_sample(&triangle,u0,u1,ns,pr);
	}

}
#include "primitive.h"
INLINE float shape_pdf(shape_info_t shape_info,cl_scene_info_t scene_info,const point3f_t* p,const vector3f_t* wi)
{
	if(shape_info.shape_type == TRIANGLE_VERTEX8)
	{
		differential_geometry_t dg_light;
		ray_t ray;
		rinit(ray,*p,*wi);
		float thit,epsilon;
		triangle_t triangle;
		unsigned memory_start = shape_info.memory_start;
		unsigned mesh_memory_start = as_uint(scene_info.shape_data[memory_start]);
		unsigned triangle_index = as_uint(scene_info.shape_data[memory_start + 1]);
		load_triangle_vertex8(scene_info.shape_data,mesh_memory_start,triangle_index,&triangle);
		if(!intersect_triangle(&triangle,&ray,&thit,&dg_light,&epsilon))
		{return 0.f;}

		vector3f_t v_tmp;
		vsub(v_tmp,dg_light.p,*p);
		float dsqr = vdot(v_tmp,v_tmp);
		float wod = fabs(vdot(dg_light.nn,*wi));
		float down = (wod * triangle_area(&triangle));
		if (down == 0.f) return 0.f;
		return dsqr / down;
	}

	return 0.f;
}
#endif
