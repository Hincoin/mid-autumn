#ifndef _LIGHT_FUNCS_H_
#define _LIGHT_FUNCS_H_

#include "light.h"
#include "cl_scene.h"
#include "shape_funcs.h"
#include "random_number_generator.h"

INLINE void light_l(cl_scene_info_t scene_info, material_info_t light_material,const point3f_t p,const normal3f_t n,const vector3f_t* wi,spectrum_t *c){
	//todo
	if(vdot(n,*wi) > 0)
	{
		unsigned texture_type = as_uint(*(scene_info.material_data+light_material.memory_start));
		unsigned memory_start = as_uint(*(scene_info.material_data+light_material.memory_start+1));
		GLOBAL float* mem = scene_info.texture_data + memory_start;
		(*c).x = mem[0];
		(*c).y = mem[1];
		(*c).z = mem[2];
	}
	else
		vclr((*c));

}


INLINE void light_sample_l(cl_scene_info_t scene_info,const light_info_t* light,
					const point3f_t *p,
					Seed* seed,
					const normal3f_t *n, float u1, float u2,
					spectrum_t* clr,
					vector3f_t *wo, float *pdf,ray_t* shadow_ray)
{
	//
	vclr(*clr);
	switch(light->light_type)
	{
	case 0:
		{
			//
			GLOBAL float* data = scene_info.light_data + light->memory_start;
			area_light_t l;
			l.primitive_idx = as_uint(data[0]);
			(*shadow_ray).o = *p;
			material_info_t light_material = scene_info.primitives[l.primitive_idx].material_info;
			unsigned st = scene_info.primitives[l.primitive_idx].shape_info.shape_type;
			//unsigned ms = scene_info.primitives[l.primitive_idx].shape_info.memory_start;

			switch (st)
			{
			case TRIANGLE_VERTEX8:
				{
					point3f_t pr;
					normal3f_t ns;
					shape_sample_on_shape(scene_info.primitives[l.primitive_idx].shape_info,scene_info,u1,u2,&ns,&pr);
					vsub((*wo),(pr),(*p));
					(*shadow_ray).d = *wo;
					(*shadow_ray).maxt = 1.f-EPSILON;
					(*shadow_ray).mint =	EPSILON;
					vnorm((*wo));
					*pdf = shape_pdf(scene_info.primitives[l.primitive_idx].shape_info,scene_info,p,wo);
					switch(light_material.material_type)
					{
					case 0:
						{
							//todo
							//load light color;
							//color_texture_info_t intensity;
							vector3f_t wi_neg;
							vneg(wi_neg,*wo);
							light_l(scene_info,light_material,pr,ns,&wi_neg,clr);
						}
						break;
					default:break;
					}
				}
			default:
				break;
			}
		}
		break;
	default:
		break;
	}

}


INLINE float light_pdf(cl_scene_info_t scene_info,const light_info_t *light,const point3f_t *p,const normal3f_t *n,
					   const vector3f_t *wi)
{
	//todo
	//shape_pdf
	if(light->light_type == 0)
	{
		area_light_t lght;
		lght.primitive_idx = as_uint((scene_info.light_data+light->memory_start)[0]);
		shape_info_t shape_info = (scene_info.primitives[lght.primitive_idx].shape_info);
		return shape_pdf(shape_info,scene_info,p,wi);
	}
	return 0;						
}
INLINE bool light_is_delta_light(cl_scene_info_t scene_info,const light_info_t* light)
{
	//todo
	return false;
}

INLINE void light_le(cl_scene_info_t scene_info,const intersection_t* isect,const vector3f_t* wi,spectrum_t* c)
{
	//todo
	if(scene_info.primitives[isect->primitive_idx].material_info.material_type == 0)//light type
	{
		light_l(scene_info,scene_info.primitives[isect->primitive_idx].material_info,isect->dg.p,isect->dg.nn,wi,c);//vinit(*c,1.f,1.f,1.f);
	}
}
#include "shape_funcs.h"
INLINE void light_power(light_info_t* light,cl_scene_info_t scene_info,spectrum_t *e)
{
	switch(light->light_type)
	{
	case 0://area light
		{
			area_light_t lght;
			lght.primitive_idx = as_uint((scene_info.light_data+light->memory_start)[0]);
			material_info_t light_material = scene_info.primitives[lght.primitive_idx].material_info;	
			spectrum_t lemit;
			switch(light_material.material_type)
			{
			case 0:
				{
					//load_color(scene_info.material_data+light_material.memory_start,&lemit);

					unsigned texture_type = as_uint(*(scene_info.material_data+light_material.memory_start));
					unsigned memory_start = as_uint(*(scene_info.material_data+light_material.memory_start+1));
					GLOBAL float* mem = scene_info.texture_data + memory_start;
					lemit.x = mem[0];
					lemit.y = mem[1];
					lemit.z = mem[2];
				}
				break;
			default:vclr(lemit);break;
			}
			shape_info_t shape_info = scene_info.primitives[lght.primitive_idx].shape_info;
			vsmul(*e,shape_area(shape_info,scene_info) * FLOAT_PI,lemit);
		}

		break;
	default:
		vclr(*e);
	}
}

INLINE void light_ray_sample_l(GLOBAL light_info_t* light,cl_scene_info_t scene_info,float u0,float u1,float u2,float u3,
			ray_t *ray,normal3f_t *ns,float *pdf,spectrum_t *alpha)
{
	//
	if(light->light_type == 0)
	{
		area_light_t lght;
		lght.primitive_idx = as_uint((scene_info.light_data+light->memory_start)[0]);
		shape_info_t shape_info = (scene_info.primitives[lght.primitive_idx].shape_info);
		shape_sample_on_shape(shape_info,scene_info,u0,u1,ns,&(ray->o));
		UniformSampleSphere(u2, u3,&(ray->d));
		if (vdot(ray->d, *ns) < 0.f) vsmul(ray->d , -1, ray->d);
		*pdf = /*shape_pdf(shape_info,&ray->o)*/(1.f/shape_area(shape_info,scene_info)) * INV_TWOPI;
		rinit(*ray,ray->o,ray->d);
		return light_l(scene_info,scene_info.primitives[lght.primitive_idx].material_info,ray->o, *ns, &ray->d,alpha);
	}

}
#endif