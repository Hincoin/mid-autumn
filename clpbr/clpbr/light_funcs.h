#ifndef _LIGHT_FUNCS_H_
#define _LIGHT_FUNCS_H_

#include "light.h"
#include "cl_scene.h"
#include "shape_funcs.h"

INLINE void light_l(const point3f_t p,const normal3f_t n,const vector3f_t* wi,spectrum_t *c){
	//todo
	vinit(*c,10.f,10.f,10.f);
}


static void load_area_light_material(GLOBAL float* light_data,
									 GLOBAL const light_info_t* lght,light_material_t* m)
{
	if (lght->light_type == 0)
	{
		area_light_t lght;
		lght.primitive_idx = as_uint(light_data[0]);
	}
}

INLINE void uniform_sampling_area_light(
										cl_scene_info_t scene_info,
										const light_info_t* light,
										Seed* seed,
										const intersection_t* isect,
										spectrum_t *c
										)
{
	//
	GLOBAL float* data = scene_info.light_data + light->memory_start;
	area_light_t l;
	l.primitive_idx = as_uint(data[0]);
	vector3f_t unit_sphere_point;
	ray_t shadow_ray ;
	shadow_ray.o = isect->dg.p;
	material_info_t light_material = scene_info.primitives[l.primitive_idx].material_info;
	unsigned st = scene_info.primitives[l.primitive_idx].shape_info.shape_type;
	unsigned ms = scene_info.primitives[l.primitive_idx].shape_info.memory_start;
	switch (st)
	{
	case 0:
		{
			UniformSampleSphere(random_float(seed),random_float(seed),&unit_sphere_point);
			sphere_t s;
			load_sphere(scene_info.shape_data + ms,&s);
			point3f_t sp;
			vsmul(unit_sphere_point,s.rad,unit_sphere_point);
			transform_point(sp,s.o2w,unit_sphere_point);
			vsub(shadow_ray.d,sp,shadow_ray.o);
			const float len = sqrt(vdot(shadow_ray.d,shadow_ray.d));
			vsmul(shadow_ray.d, 1.f/len,shadow_ray.d);

			shadow_ray.maxt = len-EPSILON;
			shadow_ray.mint =	EPSILON;

			float wo = vdot(shadow_ray.d, unit_sphere_point);
			if(wo > 0)
				return;
			else 
				wo = -wo;

			/* visibility */
			const float wi = vdot(shadow_ray.d, isect->dg.nn);

			if( (wi > 0) && !intersectP(scene_info,&shadow_ray) )
			{
				//pdf * emission	
				spectrum_t e;
				switch(light_material.material_type)
				{
				case 0:
					{
						spectrum_t e; 
						unsigned offset = load_color(scene_info.material_data+light_material.memory_start,&e);
						const float s_decay = (4.f * FLOAT_PI * s.rad * s.rad) * wi * wo / (len *len);
						vsmul(e, s_decay, e);
						vadd(*c, *c, e);	
					}
					break;
				default:break;
				}
			}
		}
	default:
		break;
	}
}
INLINE void uniform_sampling_light(
								   cl_scene_info_t scene_info,
								   Seed* seed,
								   const intersection_t* isect,
								   spectrum_t *c
								   )
{
	//
	vclr(*c);
	for (unsigned i = 0;i < scene_info.lght_count; ++i)
	{
		const light_info_t lght = scene_info.lghts[i];
		switch(lght.light_type)
		{
		case 0://area light
			uniform_sampling_area_light(scene_info,&lght,seed,isect,c);
			break;
		default:break;
		}
	}
}
void light_sample_l(cl_scene_info_t scene_info,const light_info_t* light,
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
			vector3f_t unit_sphere_point;
			(*shadow_ray).o = *p;
			material_info_t light_material = scene_info.primitives[l.primitive_idx].material_info;
			unsigned st = scene_info.primitives[l.primitive_idx].shape_info.shape_type;
			unsigned ms = scene_info.primitives[l.primitive_idx].shape_info.memory_start;
			switch (st)
			{
			case 0:
				{
					vector3f_t v_unit_sp;
					UniformSampleSphere(random_float(seed),random_float(seed),&v_unit_sp);
					sphere_t s;
					load_sphere(scene_info.shape_data + ms,&s);
					point3f_t sp;
					vsmul(unit_sphere_point,s.rad,v_unit_sp);
					transform_point(sp,s.o2w,unit_sphere_point);
					transform_vector(unit_sphere_point,s.o2w,v_unit_sp);

					vsub(*wo,sp,(*shadow_ray).o);
					const float len = sqrt(vdot(*wo,*wo));
					vsmul(*wo, 1.f/len,*wo);


					(*shadow_ray).maxt = len-EPSILON;
					(*shadow_ray).mint =	EPSILON;

					float wod = vdot(*wo, v_unit_sp);
					if(wod > 0)
						return;
					else 
						wod = -wod;

					/* visibility */
					const float wid = vdot(*wo, *n);

					if(wid > 0){
						(*shadow_ray).d = *wo;
						//shape pdf
						vector3f_t v_tmp ;
						point3f_t p,pcenter;vinit(pcenter,0,0,0);
						transform_point(p,s.o2w,pcenter);
						vsub(v_tmp,(*shadow_ray).o,p);
						float dist_sqr = vdot(v_tmp,v_tmp);
						//inside sphere
						if (dist_sqr - s.rad * s.rad < 0.00001f)
						{
							*pdf = (len *len) / ((4.f * FLOAT_PI * s.rad * s.rad)  * wod) ;
						}
						else
						{
							float cosThetaMax = sqrt(max(0.f, (1.f - s.rad * s.rad /
								dist_sqr)));
							*pdf = 1.f / (2.f * FLOAT_PI * (1.f - cosThetaMax));//uniformconepdf
						}

						switch(light_material.material_type)
						{
						case 0:
							{
								load_color(scene_info.material_data+light_material.memory_start,clr);
							}
							break;
						default:break;
						}
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
		lght.primitive_idx = as_uint(scene_info.light_data[0]);
		sphere_t s;
		load_sphere(scene_info.shape_data + scene_info.primitives[lght.primitive_idx].shape_info.memory_start
			,&s);
		vector3f_t v_tmp ;
		point3f_t p_tmp,pcenter;vinit(pcenter,0,0,0);
		transform_point(p_tmp,s.o2w,pcenter);
		vsub(v_tmp,*p,p_tmp);
		float dist_sqr = vdot(v_tmp,v_tmp);
		//inside sphere
		if (dist_sqr - s.rad * s.rad < 0.00001f)
		{
			ray_t r;rinit(r,*p,*wi);
			intersection_t isect;
			float thit;
			differential_geometry_t dg;
			if(!(intersect_sphere(scene_info.shape_data,
				scene_info.primitives[lght.primitive_idx].shape_info.memory_start,
				&r,&thit,&dg)))
			{
				return 0.f;
			}
			float wod = fabs(vdot(dg.nn,*wi));
			vsub(v_tmp,dg.p,*p);
			float dsqr = vdot(v_tmp,v_tmp);
			return (dsqr) / (4.f * FLOAT_PI * s.rad * s.rad)  * wod ;
		}
		else
		{
			float cosThetaMax = sqrt(max(0.f, (1.f - s.rad * s.rad /
				dist_sqr)));
			return 1.f / (2.f * FLOAT_PI * (1.f - cosThetaMax));
		}
	}
	return 0;						
}
INLINE bool light_is_delta_light(cl_scene_info_t scene_info,const light_info_t* light)
{
	//todo
	return false;
}

static void light_le(cl_scene_info_t scene_info,const intersection_t* isect,const vector3f_t* wi,spectrum_t* c)
{
	//todo
	if(scene_info.primitives[isect->primitive_idx].material_info.material_type == 0)//light type
		light_l(isect->dg.p,isect->dg.nn,wi,c);//vinit(*c,1.f,1.f,1.f);
}
#endif