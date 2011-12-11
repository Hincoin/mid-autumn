
#pragma OPENCL EXTENSION cl_amd_printf : enable


#include "cl_config.h"
#include "geometry.h"
#include "shape_funcs.h"
#include "primitive_funcs.h"
#include "ray.h"
#include "photon_intersection_data.h"
#include "permuted_halton.h"

__kernel void photon_intersect(__global photon_intersection_data_t *intersections,
		GLOBAL ray_t *photon_rays,
		GLOBAL float* light_data,
		GLOBAL float* material_data,
		GLOBAL float* shape_data,
		GLOBAL float* texture_data,
		GLOBAL float* accelerator_data,
		GLOBAL primitive_info_t* primitives,
		GLOBAL light_info_t* lghts, 
		CONSTANT float *lights_power,  //constant
		CONSTANT float *light_cdf, //constatt
		CONSTANT permuted_halton16_t *halton, //constant
		const float light_total_power,
		const unsigned int primitive_count, 
		const unsigned int lght_count,
		const unsigned int total_shot,
		const unsigned int number_work_items
)		
{
	const int i = get_global_id(0);
	if(i < number_work_items)
	{
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
		float u[12];
		ray_t photon_ray = photon_rays[i];
		photon_intersection_data_t other_data = intersections[i];
		spectrum_t alpha = other_data.alpha;

		intersection_t isect;
		spectrum_t ltranmittance;
		int try_count = 0;
		const int max_try_count = 2;

		if(other_data.continue_trace == 1)
		{
			if(!intersect(accelerator_data, shape_data, primitives, primitive_count,&photon_ray,&isect)) 
			{
				other_data.continue_trace = 0;
				other_data.n_intersections = 0;
			}
			else
				halton16_sample(halton,total_shot + i + 1, u, 9);
		}

		if(other_data.continue_trace == 0)
		{
			while( try_count ++ < max_try_count)
			{
				int local_shot = total_shot + i + try_count;
				halton16_sample(halton,local_shot, u, 9);
				//choose light of shoot photon from
				float lpdf;

				int lnum = (int)floor(sample_step_1d(lights_power,light_cdf,
					light_total_power,lght_count,u[0],&lpdf));
				lnum = min(lnum, (int)lght_count - 1);

				GLOBAL light_info_t* light = lghts+lnum;
				float pdf;

				normal3f_t nl;
				light_ray_sample_l(light,scene_info,u[1],u[2],u[3],u[4],
					&photon_ray,&nl,&pdf,&alpha);
				if (pdf == 0.f || color_is_black(alpha))continue;
				vsmul(alpha,(fabs(vdot(nl,photon_ray.d))/(pdf*lpdf)),alpha);
				other_data.n_intersections = 0;
				if(intersect(accelerator_data, shape_data, primitives, primitive_count,&photon_ray,&isect)) 
				{
					break;
				}
			}
			if(try_count >= max_try_count){
				intersections[i] = other_data;
				return;
			}
		}

		other_data.specular_path = other_data.n_intersections == 0 || other_data.specular_path == 1;

		++other_data.n_intersections ;

		scene_tranmittance(scene_info,&photon_ray,&ltranmittance);

		vector3f_t wo ; 
		vneg(wo,photon_ray.d);
		bsdf_t photon_bsdf;

		intersection_get_bsdf(&isect,scene_info,&photon_ray,
					   &photon_bsdf);
		BxDFType specular_type = 
		(BxDFType)(BSDF_REFLECTION|BSDF_TRANSMISSION|BSDF_SPECULAR);
		other_data.has_non_specular = (
							   photon_bsdf.n_bxdfs > bsdf_num_components(&photon_bsdf,specular_type)
							   );
	

		if (other_data.has_non_specular)
		{
			//deposit photon at surface
			photon_init(&other_data.photon,&isect.dg.p,&alpha,&wo);

			//store data for radiance photon
			normal3f_t n = isect.dg.nn;
			if(vdot(n,photon_ray.d) > 0.f)vneg(n,n);
			radiance_photon_init(&other_data.radiance_photon,&(isect.dg.p),(&n));
			bsdf_rho_hh(&photon_bsdf,&other_data.seed,BSDF_ALL_REFLECTION,&other_data.rho_r);
			bsdf_rho_hh(&photon_bsdf,&other_data.seed,BSDF_ALL_TRANSMISSION,&other_data.rho_t);
		}

		//sample new photon ray direction
		vector3f_t wi;
		float pdf;
		BxDFType flags;

		//compute new photon weight and possibly terminate with rr
		spectrum_t fr;
		bsdf_sample_f(&photon_bsdf,&wo,&wi,u[5],u[6],u[7],&pdf,BSDF_ALL,&flags,&fr);
		if(color_is_black(fr) || pdf == 0.f)
		{
			other_data.continue_trace = 0;
			intersections[i] = other_data;
			return;
		}
		spectrum_t anew;
		vmul(anew,alpha,fr);
		vsmul(anew,fabs(vdot(wi,photon_bsdf.dg_shading.nn))/pdf,anew);
		float continue_prob = min(1.f,spectrum_y(&anew)/spectrum_y(&alpha));
		if (u[8] > continue_prob  )
		{
			other_data.continue_trace = 0;
			intersections[i] = other_data;
			return;
		}
		vsmul(alpha,1.f/continue_prob,anew);
		other_data.alpha = alpha;
		other_data.specular_path = (other_data.specular_path)
			&& ((flags & BSDF_SPECULAR) != 0);

		if(other_data.n_intersections > 4)
		{
			other_data.continue_trace = 0;
		}
		else
			other_data.continue_trace = 1;
		intersections[i] = other_data;
		
		rinit(photon_ray,isect.dg.p,wi);
		photon_ray.mint = isect.ray_epsilon;
		photon_rays[i] = photon_ray;
		/*
	////////////////////////////////////////////////
		*/
	}
}
