#ifndef _SPPM_H_
#define _SPPM_H_

#include <math.h>
#include "spectrum.h"
#include "geometry.h"
#include "sampling.h"
#include "random_number_generator.h"
#include "primitive_funcs.h"
#include "integrator_funcs.h"
#include "ray_hit_point.h"


typedef struct{
	spectrum_t flux;
}photon_hit_point_t;

typedef struct  
{
	unsigned num_hash;
	unsigned* conflict_size;//each element indicates the size of array of same hash value
	ray_hit_point_t* hash_grid;
}sppm_hash_table;


typedef struct  
{
	//hash_table
}sppm_t;



INLINE bool is_hit_light(intersection_t* isect,cl_scene_info_t scene_info)
{
	return (scene_info.primitives[isect->primitive_idx].material_info.material_type == 0);//light type
}
INLINE void sppm_ray_trace(
						   GLOBAL float* light_data,GLOBAL float* material_data,GLOBAL float* shape_data,
						   GLOBAL float* texture_data,GLOBAL float* integrator_data,GLOBAL float* accelerator_data,
						   GLOBAL primitive_info_t* primitives,const unsigned int primitive_count,
						   GLOBAL light_info_t* lghts, const unsigned int lght_count,
						   const ray_t* ray,
						   Seed* seed,
						   ray_hit_point_t *hp
						   )
{
	ray_t cur_ray;rassign(cur_ray, *ray);
	vector3f_t color;vinit(color,0,0,0);
	spectrum_t throughtput; vinit(throughtput,1,1,1);

	unsigned int depth = 0;
	const unsigned int max_depth = 6;
	const unsigned int rr_depth = 5;
	intersection_t isect;
	cl_scene_info_t scene_info;
	scene_info.accelerator_data = accelerator_data;
	scene_info.material_data = material_data;
	scene_info.lght_count = lght_count;
	scene_info.lghts = lghts;
	scene_info.light_data = light_data;
	scene_info.primitive_count = primitive_count;
	scene_info.primitives = primitives;
	scene_info.shape_data = shape_data;
	scene_info.texture_data = texture_data;
	scene_info.integrator_data = integrator_data;

	int specular_bounce = 1;

	bool is_debug_ray = false;
	for (;;++depth)
	{
		if (depth > max_depth ||!intersect(accelerator_data, shape_data, primitives, primitive_count, &cur_ray,&isect) )
		{
			hp->type = hp_constant_color;
			vclr(hp->throughput);
			return;
		}

		if (depth > rr_depth && !specular_bounce)
		{
			float continue_prob = .5f;
			if (random_float(seed) > continue_prob)
			{
				hp->type = hp_constant_color;
				vclr(hp->throughput);
				return;
			}
			continue_prob = 1.f/continue_prob;
			vsmul(throughtput,continue_prob,throughtput);
		}
		if (depth == 0 || specular_bounce)
		{
			spectrum_t tmp;
			vector3f_t v;
			vneg(v,cur_ray.d);
			vclr(tmp)
			intersection_le(&isect,scene_info,&v,&tmp);
			vmul(tmp,throughtput,tmp);
			if(is_hit_light(&isect,scene_info))
			{
				hp->type = hp_constant_color;
				hp->throughput = tmp;
				return;
			}
		}
		bsdf_t bsdf;

		//compute
		intersection_get_bsdf(&isect,scene_info,&cur_ray,&bsdf);//do following
		//compute_differential_geometry(isect.dg,ray);
		//differential_geometry dgs;
		//get_dg_shading_geometry(shape,dg,&dgs)//dgs = dg;
		//material_get_bsdf(scene,material,dg,dgs,&bsdf);
		point3f_t p = bsdf.dg_shading.p;
		normal3f_t n = bsdf.dg_shading.nn;
		vector3f_t wo;vneg(wo,cur_ray.d);

		float bs1 = random_float(seed);
		float bs2 = random_float(seed);
		float bcs = random_float(seed);
		vector3f_t wi;
		float pdf;
		BxDFType flags;
		spectrum_t f;
		bsdf_sample_f(&bsdf,&wo,&wi,bs1,bs2,bcs,&pdf,BSDF_ALL,&flags,&f);
		if (pdf <= 0.f || color_is_black(f))
		{
			hp->type = hp_constant_color;
			vclr(hp->throughput);
			break;
		}
		specular_bounce = (flags & BSDF_SPECULAR) != 0;
		float co = fabs(vdot(wi,n)) / pdf;
		vmul(throughtput,throughtput,f);
		vsmul(throughtput,co,throughtput);


		if (!specular_bounce)
		{
			hp->type = hp_surface;
			hp->bsdf = bsdf;
			hp->pos = p;
			hp->normal = n;
			hp->wo = wo;
			hp->throughput = throughtput;
			return;
		}


		cur_ray.o = p;
		cur_ray.d = wi;
		cur_ray.mint = EPSILON;
		cur_ray.maxt = FLT_MAX;
	}
}
INLINE void sppm_photon_trace(
						   GLOBAL float* light_data,GLOBAL float* material_data,GLOBAL float* shape_data,
						   GLOBAL float* texture_data,GLOBAL float* integrator_data,GLOBAL float* accelerator_data,
						   GLOBAL primitive_info_t* primitives,const unsigned int primitive_count,
						   GLOBAL light_info_t* lghts, const unsigned int lght_count,
						   const ray_t* ray,
						   Seed* seed,
						   const spectrum_t* photon_throughput
							  )
{
	ray_t cur_ray;rassign(cur_ray, *ray);
	vector3f_t color;vinit(color,0,0,0);
	vector3f_t throughtput; vassign(throughtput,*photon_throughput);

	unsigned int depth = 0;
	const unsigned int max_depth = 6;
	const unsigned int rr_depth = 5;
	intersection_t isect;
	cl_scene_info_t scene_info;
	scene_info.accelerator_data = accelerator_data;
	scene_info.material_data = material_data;
	scene_info.lght_count = lght_count;
	scene_info.lghts = lghts;
	scene_info.light_data = light_data;
	scene_info.primitive_count = primitive_count;
	scene_info.primitives = primitives;
	scene_info.shape_data = shape_data;
	scene_info.texture_data = texture_data;
	scene_info.integrator_data = integrator_data;

	int specular_bounce = 1;

	bool is_debug_ray = false;
	for (;;++depth)
	{
		if(!specular_bounce)
		{
			//todo
			//hp->accum_flux += throughtput;
		}
		if (depth > max_depth ||!intersect(accelerator_data, shape_data, primitives, primitive_count, &cur_ray,&isect) )
		{
			return;
		}
		if (depth > rr_depth )
		{
			float continue_prob = .5f;
			if (random_float(seed) > continue_prob)
			{
				return;
			}
			continue_prob = 1.f/continue_prob;
			vsmul(throughtput,continue_prob,throughtput);
		}
	
		bsdf_t bsdf;

		//compute
		intersection_get_bsdf(&isect,scene_info,&cur_ray,&bsdf);//do following
		//compute_differential_geometry(isect.dg,ray);
		//differential_geometry dgs;
		//get_dg_shading_geometry(shape,dg,&dgs)//dgs = dg;
		//material_get_bsdf(scene,material,dg,dgs,&bsdf);
		point3f_t p = bsdf.dg_shading.p;
		normal3f_t n = bsdf.dg_shading.nn;
		vector3f_t wo;vneg(wo,cur_ray.d);

		float bs1 = random_float(seed);
		float bs2 = random_float(seed);
		float bcs = random_float(seed);
		vector3f_t wi;
		float pdf;
		BxDFType flags;
		spectrum_t f;
		bsdf_sample_f(&bsdf,&wo,&wi,bs1,bs2,bcs,&pdf,BSDF_ALL,&flags,&f);
		if (pdf <= 0.f || color_is_black(f))
		{
			break;
		}
		specular_bounce = (flags & BSDF_SPECULAR) != 0;
		float co = fabs(vdot(wi,n)) / pdf;
		vmul(throughtput,throughtput,f);
		vsmul(throughtput,co,throughtput);

		cur_ray.o = p;
		cur_ray.d = wi;
		cur_ray.mint = EPSILON;
		cur_ray.maxt = FLT_MAX;
	}
}
#endif