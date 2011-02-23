#ifndef _PATH_TRACING_H_
#define _PATH_TRACING_H_


#include "config.h"

#include "primitive.h"
#include "reflection.h"
#include "cl_scene.h"
#include "primitive_funcs.h"
#include "light_funcs.h"

#include "mc.h"



void estimate_direct(   cl_scene_info_t scene_info,
					 const light_info_t* light,
					 Seed* s,
							  const point3f_t* p,const normal3f_t* n,
							  const vector3f_t* wo,
							  bsdf_t* bsdf,							  
							  spectrum_t *c
							  )
{
	vclr(*c);
	float ls1 = random_float(s);
	float ls2 = random_float(s);
	float bs1 = random_float(s);
	float bs2 = random_float(s);
	float bcs = random_float(s);

	vector3f_t wi;
	float lightpdf=0,bsdfpdf=0;
	ray_t shadow_ray;
	spectrum_t li;
	light_sample_l(scene_info,light,p,s,n,ls1,ls2,&li,&wi,&lightpdf,&shadow_ray);
	if (lightpdf > 0.f && !color_is_black(li))
	{
		spectrum_t f;
		bsdf_f(bsdf,wo,&wi,BSDF_ALL,&f);
		if (!color_is_black(f) && !intersectP(scene_info,&shadow_ray))
		{

			spectrum_t tmp;
			vmul(tmp,f,li);
			float wt= fabs(vdot(wi,*n))/lightpdf;
			vsmul(tmp,wt,tmp );
			if (light_is_delta_light(scene_info,light))
			{
				//ld += f * Li * absdot(wi,n) / light_pdf;
				vadd(*c,*c,tmp);
			}
			else
			{
				bsdfpdf = bsdf_pdf(bsdf,wo,&wi,BSDF_ALL);
				float weight = power_heuristic(1,lightpdf,1,bsdfpdf);
				//ld += f * Li * absdot(wi,n)*weight /lightpdf;
				vsmul(tmp,weight,tmp)	;
				vadd(*c,*c,tmp);
			}
		}
	}
	if (!light_is_delta_light(scene_info,light))
	{
		BxDFType flags =(BxDFType) (BSDF_ALL & ~BSDF_SPECULAR);
		spectrum_t f;
		BxDFType sampled_flags;
		bsdf_sample_f(bsdf,wo,&wi,bs1,bs2,bcs,&bsdfpdf,flags,&sampled_flags,&f);
		if (bsdfpdf > 0.f && !color_is_black(f))
		{
			lightpdf = light_pdf(scene_info,light,p,n,&wi);
			if(lightpdf > 0.f)
			{
				intersection_t light_isect;
				ray_t ray;
				rinit(ray,*p,wi);
				vclr(li);
				if (intersect(scene_info.accelerator_data,scene_info.shape_data,scene_info.primitives,scene_info.primitive_count,
					&ray,&light_isect))
				{
					unsigned primitive_idx = -1;
					switch(light->light_type)
					{
					case 0:
						{
						GLOBAL float* data = scene_info.light_data + light->memory_start;
						//area_light_t 
						primitive_idx = as_uint(data[0]);
						}
						break;
					default:
						break;
					}
					if(light_isect.primitive_idx == primitive_idx)
					{
						vneg(wi,wi);
						if (vdot(wi,light_isect.dg.nn) > 0.f)
						{
							//light_le(scene_info,&light_isect,&li);
							intersection_le(&light_isect,scene_info,&wi,&li);
						}
					}
				}
				else
				{
					//todo
					//black
					//light_le(scene_info,light,ray,&li);
					vclr(li);
				}
				if(!color_is_black(li))
				{
					float weight = power_heuristic(1,bsdfpdf,1,lightpdf);
					spectrum_t tmp;
					weight *= (fabs(vdot(wi,*n)) /bsdfpdf);
					vmul(tmp,f,li);
					vsmul(tmp,weight,tmp);
					vadd(*c,tmp,*c);
				//	Ld += f * Li * AbsDot(wi, n) * weight / bsdfPdf
				}
			}
		}
	}
}
void uniform_sample_all_lights(  cl_scene_info_t scene_info,
							   Seed* seed,
							   const point3f_t* p,const normal3f_t* n,
							   const vector3f_t* wo,
							   bsdf_t* bsdf,							  
							   spectrum_t *c)
{
	//
	vclr(*c);
	for (unsigned i = 0;i < scene_info.lght_count; ++i)
	{
		const light_info_t lght = scene_info.lghts[i];
		spectrum_t ld;
		estimate_direct(scene_info,&lght,seed,p,n,wo,bsdf,&ld);
		vadd(*c,*c,ld);
	}
}



static void PathTracing(
GLOBAL float* light_data,GLOBAL float* material_data,GLOBAL float* shape_data,
GLOBAL float* texture_data,GLOBAL float* integrator_data,GLOBAL float* accelerator_data,
GLOBAL primitive_info_t* primitives,const unsigned int primitive_count,
GLOBAL light_info_t* lghts, const unsigned int lght_count,
const ray_t* ray,
Seed* seed,
vector3f_t *result
)
{
	ray_t cur_ray;rassign(cur_ray, *ray);
	vector3f_t color;vinit(color,0,0,0);
	vector3f_t throughtput; vinit(throughtput,1,1,1);

	unsigned int depth = 0;
	const unsigned int max_depth = 3;
	const unsigned int rr_depth = 3;
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
		if (depth > max_depth)
		{
			*result = color;
			return;
		}
		if (!intersect(accelerator_data, shape_data, primitives, primitive_count, &cur_ray,&isect))
		{
			*result = color;
			return;
		}

		if (depth > rr_depth)
		{
			float continue_prob = .5f;
			if (random_float(seed) > continue_prob)
			{
				*result = color;
				return;
			}
			continue_prob = 1.f/continue_prob;
			vsmul(throughtput,continue_prob,throughtput);
		}
		if (depth == 0)
		{
			//ray->maxt = cur_ray.maxt;
		}
		else 
		{
			//path_throughput *= transmittance(ray);
		}

		if (depth == 0 || specular_bounce)
		{
			spectrum_t tmp;
			vector3f_t v;
			vneg(v,cur_ray.d);
			vclr(tmp)
			intersection_le(&isect,scene_info,&v,&tmp);
			vmul(tmp,throughtput,tmp);
			vadd(color,color,tmp);
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

		if(isect.primitive_idx == 7 && depth == 0)
		{
			is_debug_ray = true;
		}
		if (is_debug_ray && depth == 2)
		{
			//printf("debug depth=2");
		}
		
		spectrum_t c_tmp;
		uniform_sample_all_lights(scene_info,seed,&p,&n,&wo,&bsdf,&c_tmp);
		vmul(c_tmp,c_tmp,throughtput);
		vadd(color,color,c_tmp);

		if (is_debug_ray && depth == 3)
		{
			//printf("debug depth=3 color_is_black:%d \n",color_is_black(c_tmp));
		}

		float bs1 = random_float(seed);
		float bs2 = random_float(seed);
		float bcs = random_float(seed);
		vector3f_t wi;
		float pdf;
		BxDFType flags;
		spectrum_t f;
		bsdf_sample_f(&bsdf,&wo,&wi,bs1,bs2,bcs,&pdf,BSDF_ALL,&flags,&f);
		if (pdf == 0.f || color_is_black(f))
		{
			*result = color;
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

		continue;
		//is this a light ?

		const float dp = vdot(isect.dg.nn, cur_ray.d);
		for (unsigned i = 0; i < scene_info.lght_count; ++i)
		{
			if(scene_info.lghts[i].light_type == 0)
			{
				//it's area light
				unsigned pi = as_uint((light_data+scene_info.lghts[i].memory_start)[0]);
				if(isect.primitive_idx == pi)
				{
					//printf("pi,isect_primitive : %d,%d\t",pi,isect.primitive_idx);
					spectrum_t e;
					light_le(scene_info,&isect,&cur_ray.d,&e);
					if (specular_bounce)
					{
						vmul(e,throughtput,e);
						vadd(color,color,e);
					}
					vassign(*result,color);
					return;
				}
			}
		}
		//todo 
		compile_test();
		//diffuse test
		specular_bounce = 0;

		//only diffuse : for reversed sphere
		vector3f_t nl;
		const float invSignDP = -1.f * sign(dp);
		vsmul(nl, invSignDP, isect.dg.nn);
		vassign(isect.dg.nn,nl);

		spectrum_t test_color;
		vinit(test_color,0.5f,0.1f,0.2f);
		vmul(throughtput, throughtput, test_color);
		spectrum_t ld;
		uniform_sampling_light(scene_info,seed,&isect,&ld);
		//printf("color(%f,%f,%f),ld(%f,%f,%f)\n",color.x,color.y,color.z,ld.x,ld.y,ld.z);
		vmul(ld, throughtput, ld);
		vadd(color,color,ld);
		/* Diffuse component */

		float r1 = 2.f * FLOAT_PI * random_float(seed);
		float r2 = random_float(seed);
		float r2s = sqrt(r2);

		vector3f_t w; vassign(w, isect.dg.nn);

		vector3f_t u, a;
		if (fabs(w.x) > .1f) {
			vinit(a, 0.f, 1.f, 0.f);
		} else {
			vinit(a, 1.f, 0.f, 0.f);
		}
		vxcross(u, a, w);
		vnorm(u);

		vector3f_t v;
		vxcross(v, w, u);

		vector3f_t newDir;
		vsmul(u, cos(r1) * r2s, u);
		vsmul(v, sin(r1) * r2s, v);
		vadd(newDir, u, v);
		vsmul(w, sqrt(1 - r2), w);
		vadd(newDir, newDir, w);

		cur_ray.o = isect.dg.p;
		cur_ray.d = newDir;
		cur_ray.mint = EPSILON;
		cur_ray.maxt = FLT_MAX;
	}
}

#endif
