
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
		GLOBAL float *lights_power,  //constant
		GLOBAL float *light_cdf, //constatt
		GLOBAL permuted_halton16_t *halton, //constant
		const float light_total_power,
		const unsigned int primitive_count, 
		const unsigned int lght_count,
		const unsigned int total_shot,
		const unsigned int number_work_items
)		
{
	const int i = get_global_id(0);
	//intersections[0].n_intersections = 100;
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
		float u[16];
		ray_t photon_ray = photon_rays[i];
		photon_intersection_data_t other_data = intersections[i];
		spectrum_t alpha = other_data.alpha;

		int try_count = 0;
		if(other_data.continue_trace == 1)
		{
			halton16_sample(halton,total_shot + i + 1, u);
			/*
			u[0] = radical_inverse(total_shot + i + 1, 2);
			u[1] = radical_inverse(total_shot + i + 1, 3);
			u[2] = radical_inverse(total_shot + i + 1, 5);
			u[3] = radical_inverse(total_shot + i + 1, 7);
			u[4] = radical_inverse(total_shot + i + 1, 11);
			u[5] = radical_inverse(total_shot + i + 1, 13);
			u[6] = radical_inverse(total_shot + i + 1, 17);
			u[7] = radical_inverse(total_shot + i + 1, 19);
			u[8] = radical_inverse(total_shot + i + 1, 23);
			*/
		}
		else
		{
			while( try_count ++ < 3)
		{
			int local_shot = total_shot + i + try_count + 1;
			/*
			u[0] = radical_inverse(local_shot, 2);
			u[1] = radical_inverse(local_shot, 3);
			u[2] = radical_inverse(local_shot, 5);
			u[3] = radical_inverse(local_shot, 7);
			u[4] = radical_inverse(local_shot, 11);
			u[5] = radical_inverse(local_shot, 13);
			u[6] = radical_inverse(local_shot, 17);
			u[7] = radical_inverse(local_shot, 19);
			u[8] = radical_inverse(local_shot, 23);
			*/
			halton16_sample(halton,local_shot, u);
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
			break;
		}
		if(try_count >= 3) return;
	
		}
		other_data.specular_path = other_data.n_intersections == 0 || other_data.specular_path == 1;

		intersection_t isect;
		spectrum_t ltranmittance;
		if(!intersect(accelerator_data, shape_data, primitives, primitive_count,&photon_ray,&isect)) 
		{
			other_data.continue_trace = 0;
			other_data.n_intersections = 0;
			intersections[i] = other_data;
			return;
		}
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
		}
		//sample new photon ray direction
		vector3f_t wi;
		float pdf;
		BxDFType flags;
		float u1,u2,u3;
		{
			u1 = u[5];//random_float(&seed);
			u2 = u[6];//random_float(&seed);
			u3 = u[7];//random_float(&seed);
			//printf("%.3f,%.3f,%.3f\t",u1,u2,u3);
		}
		//compute new photon weight and possibly terminate with rr
		spectrum_t fr;
		bsdf_sample_f(&photon_bsdf,&wo,&wi,u1,u2,u3,&pdf,BSDF_ALL,&flags,&fr);
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
		other_data.specular_path = (other_data.specular_path)
			&& ((flags & BSDF_SPECULAR) != 0);
		rinit(photon_ray,isect.dg.p,wi);
		photon_ray.mint = isect.ray_epsilon;

		if(other_data.n_intersections > 4)
		{
			other_data.continue_trace = 0;
		}
		else
			other_data.continue_trace = 1;

		photon_rays[i] = photon_ray;
		other_data.alpha = alpha;
		intersections[i] = other_data;
	}
}
