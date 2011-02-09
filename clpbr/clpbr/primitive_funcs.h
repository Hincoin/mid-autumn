#ifndef _PRIMITIVE_FUNCS_H_
#define _PRIMITIVE_FUNCS_H_

#include "primitive.h"
#include "light_funcs.h"
#include "material_funcs.h"
INLINE void intersection_le(const intersection_t *isect,cl_scene_info_t scene_info,const vector3f_t* v,spectrum_t *c)
{
	if(scene_info.primitives[isect->primitive_idx].material_info.material_type == 0)//light type
	{
		load_color(scene_info.material_data+scene_info.primitives[isect->primitive_idx].material_info.memory_start,c);
		//light_l(isect->dg.p,isect->dg.nn,v,c);
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
#endif