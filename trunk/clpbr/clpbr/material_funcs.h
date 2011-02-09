#ifndef _MATERIAL_FUNCS_H_
#define _MATERIAL_FUNCS_H_
#include "material.h"


#include "texture.h"

typedef enum {LIGHT_MATERIAL=0,MATTE_MATERIAL,GLASS_MATERIAL,MIRROR_MATERIAL}MaterialType;

typedef struct {
	color_texture_info_t kd;
	float_texture_info_t sig;
}matte_t;

INLINE void load_matte(material_info_t* mat,cl_scene_info_t* scene_info,matte_t* m)
{
	GLOBAL float* mem = scene_info->material_data + mat->memory_start;
	m->kd.texture_type =as_uint( mem[0]);
	m->kd.memory_start = as_uint(mem[1]);
	m->sig.texture_type = as_uint(mem[2]);
	m->sig.memory_start = as_uint(mem[3]);
}
typedef struct  
{
	color_texture_info_t kr,kt;
	float_texture_info_t index;
}glass_t;

INLINE void load_glass(material_info_t* mat,cl_scene_info_t* scene_info,glass_t* gls)
{
	GLOBAL float* mem = scene_info->material_data + mat->memory_start;
	gls->kr.texture_type = as_uint(mem[0]);
	gls->kr.memory_start = as_uint(mem[1]);
	
	gls->kt.texture_type = as_uint(mem[2]);
	gls->kt.memory_start = as_uint(mem[3]);

	gls->index.texture_type = as_uint(mem[4]);
	gls->index.memory_start = as_uint(mem[5]);
}
typedef struct  
{
	color_texture_info_t kr;
}mirror_t;
INLINE void load_mirror(material_info_t* mat,cl_scene_info_t* scene_info,mirror_t* mrr)
{
	GLOBAL float* mem = scene_info->material_data + mat->memory_start;
	mrr->kr.texture_type = as_uint(mem[0]);
	mrr->kr.memory_start = as_uint(mem[1]);
}

void material_get_bsdf(material_info_t* mat,cl_scene_info_t* scene_info,
					   differential_geometry_t* dg_geom,
					   differential_geometry_t* dg_shading,
						bsdf_t* bsdf
					   );

INLINE void material_get_bsdf(material_info_t* mat,cl_scene_info_t* scene_info,
							  differential_geometry_t* dg_geom,
							  differential_geometry_t* dg_shading,
							  bsdf_t* bsdf
							  )
{

	//todo: bump mapping to compute dgs
	differential_geometry_t dgs = *dg_shading;
	//////////////////////////////////////////////////////////////////////////

	bsdf_init(bsdf,&dgs,&dg_geom->nn,1.f);
	switch(mat->material_type)
	{
	case MATTE_MATERIAL:
		{
			matte_t m;
		load_matte(mat,scene_info,&m);
		//
		bxdf_t bxdf;

		spectrum_t c ;
		float sigma;
		color_texture_evaluate(&m.kd,scene_info,&dgs,&c);
		color_clamp(c,0.f,FLT_MAX);
		float_texture_evaluate(&m.sig,scene_info,&dgs,&sigma);
		if(sigma == 0.f)
		{bxdf_init_lambertian(bxdf,c);}
		else
		{bxdf_init_oren_nayar(bxdf,c,sigma);}
		bsdf_add(bsdf,&bxdf);
		}
		break;
	case GLASS_MATERIAL:
		{
			glass_t gls;
		load_glass(mat,scene_info,&gls);
		bxdf_t bxdf;
		spectrum_t r,t;
		color_texture_evaluate(&gls.kr,scene_info,&dgs,
			&r);
		color_texture_evaluate(&gls.kt,scene_info,&dgs,
			&t);
		color_clamp(r,0.f,FLT_MAX);
		color_clamp(t,0.f,FLT_MAX);
		float ior = 0;
		float_texture_evaluate(&gls.index,scene_info,&dgs,
			&ior);
		fresnel_t fr;
		if (!color_is_black(r))
		{
			fresnel_dielectric_init(fr,1.f,ior);
			bxdf_init_specular_reflection(bxdf,r,fr);
			bsdf_add(bsdf,&bxdf);
		}
		if (!color_is_black(t))
		{
			fresnel_dielectric_init(fr,1.f,ior);
			bxdf_init_specular_transmission(bxdf,t,1.f,ior,fr);
			bsdf_add(bsdf,&bxdf);
		}
		bsdf->eta = ior;
		}
		break;
	case MIRROR_MATERIAL:
		{
			//
			mirror_t m;
			load_mirror(mat,scene_info,&m);
			bxdf_t bxdf;
			spectrum_t r;
			color_texture_evaluate(&m.kr,scene_info,&dgs,&r);
			color_clamp(r,0.f,FLT_MAX);
			if (!color_is_black(r))
			{
				fresnel_t fresnel;
				fresnel_noop_init(fresnel);
				bxdf_init_specular_reflection(bxdf,r,fresnel);
				bsdf_add(bsdf,&bxdf);
			}
		}
		break;
	default:break;
	}
}
#endif