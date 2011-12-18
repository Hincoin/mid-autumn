
#pragma OPENCL EXTENSION cl_amd_printf : enable


#include "cl_config.h"
#include "geometry.h"
#include "shape_funcs.h"
#include "primitive_funcs.h"
#include "ray.h"
#include "photon_intersection_data.h"
#include "permuted_halton.h"

__kernel void photon_intersect(
		GLOBAL photon_intersection_data_t *intersections,
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
	//if(i < number_work_items)
	//for(int k = 0; k < number_work_items; ++k)
	{
		const int i = get_global_id(0);
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
		float u[5];
		photon_intersection_data_t other_data;


		ray_t photon_ray = photon_rays[i];
		other_data = intersections[i];



		spectrum_t alpha = other_data.alpha;
		//mem_fence(CLK_GLOBAL_MEM_FENCE);
		//spectrum_t ltranmittance;
		int try_count ;
		
		try_count = 0;

		if(photon_intersection_data_continue_trace(&other_data))
		{
			if(!intersect(accelerator_data, shape_data, primitives, primitive_count,&photon_ray,&other_data.isect)) 
			{
				photon_intersection_data_set_continue_trace(&other_data,0);
				photon_intersection_data_set_n_intersections(&other_data,0);
			}
			//else
			//{
			//	halton16_sample(halton,total_shot + i + 1, u , 5);
			//}
		}

		if(!photon_intersection_data_continue_trace(&other_data))
		{
			while( try_count ++ < 4)
			{
				int local_shot = total_shot + i + try_count;
				halton16_sample(halton,local_shot, u , 5);
				//choose light of shoot photon from
				float lpdf;

				int lnum = (int)floor(sample_step_1d(lights_power,light_cdf,
					light_total_power,lght_count,u[0],&lpdf));
				lnum = min(lnum, (int)lght_count - 1);

				GLOBAL light_info_t* light = lghts+lnum;
				float pdf;

				normal3f_t nl;
				light_ray_sample_l(light,scene_info,u[1 ],u[2 ],u[3],u[4],
					&photon_ray,&nl,&pdf,& alpha);
				if (pdf == 0.f || color_is_black(alpha))continue;
				vsmul(other_data.alpha,(fabs(vdot(nl,photon_ray.d))/(pdf*lpdf)),alpha);
				photon_intersection_data_set_n_intersections(&other_data,0);
				if(intersect(accelerator_data, shape_data, primitives, primitive_count,&photon_ray,&other_data.isect)) 
				{
					break;
				}
			}
			if(try_count >= 4){
				 intersections[i] = other_data;
				return;
			}
		}

		photon_intersection_data_set_specular_path(&other_data, 
				photon_intersection_data_n_intersections(&other_data) == 0 ||
				photon_intersection_data_is_specular_path(&other_data) == true);
		//if(photon_intersection_data_n_intersections(&other_data) == 1 && other_data.alpha.x < 0.000001f)
		{
		//	printf("intersect_kernel %d,%d\t",i,photon_intersection_data_continue_trace(&other_data));
		}
		//if(i == 1)
		//printf("before ++n_intersections %d\t",photon_intersection_data_n_intersections(&other_data));
		photon_intersection_data_set_continue_trace(&other_data,1);
		photon_intersection_data_set_n_intersections(&other_data, 1 + photon_intersection_data_n_intersections(&other_data));
		//other_data.n_intersections = 1 + other_data.n_intersections ;

	//	printf("other_data n_intersections : %d\n",photon_intersection_data_n_intersections(&other_data));
		//scene_tranmittance(scene_info,&photon_ray,&ltranmittance);

		mem_fence(CLK_GLOBAL_MEM_FENCE);

		 intersections[i] = other_data;
		//if(i == 1)
		//	printf("after ++n_intersections %d\t",photon_intersection_data_n_intersections(&other_data));
		 photon_rays[i] = photon_ray;

		//mem_fence(CLK_GLOBAL_MEM_FENCE);
	}
}
