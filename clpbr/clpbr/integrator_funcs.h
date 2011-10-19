#ifndef _INTEGRATOR_FUNCS_H_
#define _INTEGRATOR_FUNCS_H_



#include "primitive.h"
#include "reflection.h"
#include "cl_scene.h"
#include "primitive_funcs.h"
#include "light_funcs.h"

#include "mc.h"

INLINE void estimate_direct(   cl_scene_info_t scene_info,
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
INLINE void uniform_sample_all_lights(  cl_scene_info_t scene_info,
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



INLINE void specular_reflect(ray_t *ray, bsdf_t *bsdf,
						 Seed *seed, const intersection_t *isect, 
						 cl_scene_info_t scene_info, spectrum_t *L, ray_t *out) {
							 vector3f_t wo ,wi;
							 vneg(wo,ray->d) ;
							 float pdf;
							 point3f_t p;
							 normal3f_t n;
							 vassign(p,bsdf->dg_shading.p);
							 vassign(n,bsdf->dg_shading.nn);
							 BxDFType flags; 
							 spectrum_t f;
							 bsdf_sample_f(bsdf,&wo,&wi,random_float(seed),random_float(seed),random_float(seed),&pdf,
								 (BxDFType)(BSDF_REFLECTION | BSDF_SPECULAR),&flags,&f);
							 vclr(*L);
							 if (pdf > 0.f && !color_is_black(f) && fabs(vdot(wi, n)) != 0.f) {
								 // Compute ray differential _rd_ for specular reflection
								 rinit(*out,p,wi);
								 out->mint = isect->ray_epsilon;
								 //Spectrum Li = renderer->Li(scene, rd, sample, rng, arena);
								 //L = f * Li * AbsDot(wi, n) / pdf;
								 vsmul(*L, fabs(vdot(wi,n))/pdf,f);
							 }
}


INLINE void specular_transmit(ray_t *ray, bsdf_t *bsdf,
						  Seed *seed, const intersection_t *isect, 
						  cl_scene_info_t scene_info, spectrum_t *L,ray_t* out) {
							  vector3f_t wo,wi ;
							  vneg(wo,ray->d);
							  float pdf;
							  point3f_t p;
							  normal3f_t n;
							  vassign(p,bsdf->dg_shading.p);
							  vassign(n,bsdf->dg_shading.nn);
							  BxDFType flags; 
							  spectrum_t f;
							  bsdf_sample_f(bsdf,&wo,&wi,random_float(seed),random_float(seed),random_float(seed),&pdf,
								  (BxDFType)(BSDF_TRANSMISSION| BSDF_SPECULAR),&flags,&f);
							  vclr(*L);
							  if (pdf > 0.f && !color_is_black(f) && fabs(vdot(wi, n)) != 0.f) {
								  // Compute ray differential _rd_ for specular transmission
								 rinit(*out,p,wi);
								 out->mint = isect->ray_epsilon;
								  //Spectrum Li = renderer->Li(scene, rd, sample, rng, arena);
								  //L = f * Li * AbsDot(wi, n) / pdf;
								  vsmul(*L,fabs(vdot(wi,n))/pdf,f);
							  }
}




#endif
