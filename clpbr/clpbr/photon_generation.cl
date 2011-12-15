
#pragma OPENCL EXTENSION cl_amd_printf : enable

#include "cl_config.h"
#include "geometry.h"
#include "shape_funcs.h"
#include "primitive_funcs.h"
#include "ray.h"
#include "photon_intersection_data.h"
#include "permuted_halton.h"

__kernel void photon_generation(
		GLOBAL photon_intersection_data_t *intersections,
		GLOBAL ray_t *photon_rays,
		GLOBAL float* light_data,
		GLOBAL float* material_data,
		GLOBAL float* shape_data,
		GLOBAL float* texture_data,
		GLOBAL float* accelerator_data,
		GLOBAL primitive_info_t* primitives,
		GLOBAL light_info_t* lghts, 
		GLOBAL generated_photon_t* generated_photons,
		const unsigned int primitive_count, 
		const unsigned int lght_count,
		const unsigned int number_work_items
		)
{
	//if( i < number_work_items)
	//for(int k = 0; k < number_work_items; ++k)
	{
		const int i = get_global_id(0);//*number_work_items + k;
		cl_scene_info_t scene_info;
		scene_info.light_data = light_data;
		scene_info.material_data = material_data;
		scene_info.shape_data = shape_data;
		scene_info.texture_data = texture_data;
		scene_info.accelerator_data = accelerator_data;
		scene_info.primitives = primitives;
		scene_info.primitive_count = primitive_count;
		scene_info.lghts = lghts;
		scene_info.lght_count = lght_count;

		ray_t photon_ray = photon_rays[i];
		intersection_t isect = intersections[i].isect;
		photon_intersection_data_t other_data = intersections[i]	;
		spectrum_t alpha = other_data.alpha;
		if (photon_intersection_data_n_intersections(&other_data) < 1)
			return;
		//if(other_data.alpha.x < 0.000001f)
		//	printf("load errrrrrrrrrrrr");

	//	if(i == 1)
	//		printf("photon_generation %d",photon_intersection_data_n_intersections(&other_data));

		vector3f_t wo ; 
		vneg(wo,photon_ray.d);
		bsdf_t photon_bsdf;

		intersection_get_bsdf(&isect,scene_info,&photon_ray,
					   &photon_bsdf);
		BxDFType specular_type = 
		(BxDFType)(BSDF_REFLECTION|BSDF_TRANSMISSION|BSDF_SPECULAR);
		photon_intersection_data_set_non_specular(&other_data, (
							   photon_bsdf.n_bxdfs > bsdf_num_components(&photon_bsdf,specular_type)
							   ));
		Seed seed = other_data.seed;

		if (photon_intersection_data_has_non_specular(&other_data))
		{
		//	if(other_data.alpha.x < 0.000001f)
		//		printf("small red component when store photon\t");
			//deposit photon at surface
			photon_t photon;
			photon_init(&photon,&isect.dg.p,& alpha,&wo);
			generated_photons[i].photon = photon;

			//store data for radiance photon
			normal3f_t n = isect.dg.nn;
			radiance_photon_t radiance_photon;
			spectrum_t rho_r,rho_t;
			if(vdot(n,photon_ray.d) > 0.f)vneg(n,n);
			radiance_photon_init(&radiance_photon,&(isect.dg.p),(&n));
			bsdf_rho_hh(&photon_bsdf,&seed,BSDF_ALL_REFLECTION,&rho_r);
			bsdf_rho_hh(&photon_bsdf,&seed,BSDF_ALL_TRANSMISSION,&rho_t);
			generated_photons[i].radiance_photon = radiance_photon;
			generated_photons[i].rho_r = rho_r;
			generated_photons[i].rho_t = rho_t;
		}

		//sample new photon ray direction
		vector3f_t wi;
		float pdf;
		BxDFType flags;

		//compute new photon weight and possibly terminate with rr
		spectrum_t fr;
		float u1 = random_float(&seed);
		float u2 = random_float(&seed);
		float u3 = random_float(&seed);
		bsdf_sample_f(&photon_bsdf,&wo,&wi,u1,u2,u3,&pdf,BSDF_ALL,&flags,&fr);
		if(color_is_black(fr) || pdf == 0.f)
		{
			photon_intersection_data_set_continue_trace(&other_data,0);
			other_data.seed = seed;
			intersections[i] = other_data;
			return;
		}
		//if(fr.x < 0.000001f)
			//printf("00000000bsdf_sample_f small red component:%f,%f,%d,%f,%f,%f,pdf:%f\t",fr.x,other_data.alpha.x,photon_intersection_data_n_intersections(&other_data),u1,u2,u3,pdf);

		spectrum_t anew;
		vmul(anew, alpha,fr);
		vsmul(anew,fabs(vdot(wi,photon_bsdf.dg_shading.nn))/pdf,anew);
		//if(anew.x < 0.000001f)
			//printf("11111111bsdf_sample_f small red component:%f,%f,%d,%f,%f,%f,pdf:%f\t",anew.x,other_data.alpha.x,photon_intersection_data_n_intersections(&other_data),u1,u2,u3,pdf);

		float continue_prob = min(1.f,spectrum_y(&anew)/spectrum_y(& alpha));
		if (random_float(&seed) > continue_prob  )
		{
			photon_intersection_data_set_continue_trace(&other_data,0);
			other_data.seed = seed;
			intersections[i] = other_data;
			return;
		}
		vsmul(other_data.alpha,1.f/continue_prob,anew);
		//if(other_data.alpha.x < 0.000001f)
			//printf("2222222222bsdf_sample_f small red component:%f,%f,%d,%f,%f,%f,pdf:%f,continue:%f\t",anew.x,other_data.alpha.x,photon_intersection_data_n_intersections(&other_data),u1,u2,u3,pdf,continue_prob);

		//other_data[local_thread_id].alpha = alpha;
		photon_intersection_data_set_specular_path(&other_data, 
				photon_intersection_data_is_specular_path(&other_data) && ((flags & BSDF_SPECULAR) != 0));

		if(photon_intersection_data_n_intersections(&other_data) > 4)
		{
			photon_intersection_data_set_continue_trace(&other_data,0);
		}
		else
			photon_intersection_data_set_continue_trace(&other_data,1);
		
		other_data.seed = seed;
		intersections[i] = other_data;
		//if(i == 1)
		//	printf("photon_generation %d",photon_intersection_data_n_intersections(&other_data));

		rinit(photon_ray,isect.dg.p,wi);
		photon_ray.mint = isect.ray_epsilon;
		photon_rays[i] = photon_ray;
	}
}
