#ifndef _PRIMITIVE_FUNCS_H_
#define _PRIMITIVE_FUNCS_H_

#include "primitive.h"
#include "light_funcs.h"
#include "material_funcs.h"
#include "triangle_mesh_ocl.h"

INLINE void intersection_le(const intersection_t *isect,cl_scene_info_t scene_info,const vector3f_t* v,spectrum_t *c)
{
	if(scene_info.primitives[isect->primitive_idx].material_info.material_type == 0)//light type
	{
		light_l(scene_info,scene_info.primitives[isect->primitive_idx].material_info,isect->dg.p,isect->dg.nn,v,c);
	}
}

INLINE void	intersection_get_bsdf(intersection_t *isect,cl_scene_info_t scene_info,
								  const ray_t *cur_ray,
								  bsdf_t *bsdf)//do following
{
	//todo 
	compute_differential_geometry(isect->dg,*cur_ray);
//compute_differential_geometry(isect.dg,ray);
//differential_geometry dgs;
//get_dg_shading_geometry(shape,dg,&dgs)//dgs = dg;

	differential_geometry_t dgs;
	dgs = isect->dg;
	material_info_t mi = scene_info.primitives[isect->primitive_idx].material_info;

	/////////////////////////////////////
	material_get_bsdf(&mi,&scene_info,&dgs,&dgs,bsdf);
	return;
	switch(mi.material_type)
	{
	case MATTE_MATERIAL:
		break;
	}
	//hard code
	spectrum_t r;
	vinit(r,1,1,1);
	bxdf_init_lambertian(bsdf->bxdfs[0],r);
	//material_get_bsdf(scene,mi,dg,dgs,&bsdf); 
}

//this is a simple test without accelerator privided
static int intersect(
					 GLOBAL float* accelerator_data,
					 GLOBAL float* shape_data,
					 GLOBAL primitive_info_t* primitives,
					 const unsigned int primitive_count,
					 ray_t* r,
					 //Seed* seed,
					 intersection_t *isect
					 )
{
	//
	int ret = 0;
	float thit;
	float epsilon;
	for (unsigned int i = 0;i < primitive_count; ++i)
	{
		switch(primitives[i].shape_info.shape_type)//intersect with shape
		{
		case 0:
			if(intersect_sphere(shape_data,primitives[i].shape_info.memory_start,r,&thit,&(isect->dg),&epsilon))
			{
				ret = 1;
				isect->primitive_idx = i;
				isect->ray_epsilon = epsilon;
				r->maxt = thit;
			}
			break;
		case TRIANGLE_VERTEX8:
			{
				triangle_t triangle;
				unsigned memory_start = primitives[i].shape_info.memory_start;
				unsigned mesh_memory_start = as_uint(shape_data[memory_start]);
				unsigned triangle_index = as_uint(shape_data[memory_start + 1]);
				load_triangle_vertex8(shape_data,mesh_memory_start,triangle_index,&triangle);
				if(intersect_triangle(&triangle,r,&thit,&(isect->dg),&epsilon))
				{
					ret = 1;
					isect->primitive_idx = i;
					isect->ray_epsilon = epsilon;
					r->maxt = thit;
				}
			}
			break;
		default:
			break;
		}
	}
	return ret;
}

INLINE int intersectP(cl_scene_info_t scene,ray_t *r)
{
	for (unsigned int i = 0;i < scene.primitive_count; ++i)
	{
		switch(scene.primitives[i].shape_info.shape_type)//intersect with shape
		{
		case 0:
			if(intersect_sphereP(scene.shape_data,scene.primitives[i].shape_info.memory_start,r))
			{
				//printf("intersect test failed : %d %f,%f\n",i,r->mint,r->maxt);
				return 1;
			}
			break;
		case TRIANGLE_VERTEX8:
			{
				triangle_t triangle;
				unsigned memory_start = scene.primitives[i].shape_info.memory_start;
				unsigned mesh_memory_start = as_uint(scene.shape_data[memory_start]);
				unsigned triangle_index = as_uint(scene.shape_data[memory_start + 1]);
				load_triangle_vertex8(scene.shape_data,mesh_memory_start,triangle_index,&triangle);
				if(intersect_triangleP(&triangle,r))
				{
					return 1;
				}
			}
			break;
		default:
			break;
		}
	}

	//printf("intersect test success\n");
	return 0;
}
#endif