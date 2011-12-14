
#pragma OPENCL EXTENSION cl_amd_printf : enable

#include "cl_config.h"
#include "geometry.h"
#include "light.h"

#include "random_number_generator.h"
#include "shape_funcs.h"
#include "photon_map.h"
#include "ray.h"

//light param data
//material param data
//shape param data
//texture param data

__kernel void render(
__global spectrum_t *colors , __global Seed *seeds,

GLOBAL float* light_data,GLOBAL float* material_data,GLOBAL float* shape_data,
GLOBAL float* texture_data,GLOBAL float* integrator_data,GLOBAL float* accelerator_data,
GLOBAL primitive_info_t* primitives,
GLOBAL light_info_t* lghts,

GLOBAL ray_differential_t *ray,
const unsigned int primitive_count, const unsigned int lght_count,const unsigned int number_work_items
)
{
    const int gid = get_global_id(0);
	if(gid < number_work_items)
	{
	LOCAL ray_t ray_stack[MAX_RAY_DEPTH];
    LOCAL spectrum_t passthrough[MAX_RAY_DEPTH];
	LOCAL bsdf_t bsdf_stack[MAX_RAY_DEPTH];
	LOCAL bool left_stack[MAX_RAY_DEPTH];//todo: change to bit 
	//lphoton data
	LOCAL close_photon_t close_photon_data_store[MAX_CLOSE_PHOTON_LOOKUP];
	//final gather data
	LOCAL close_photon_t photon_buffer[n_indir_sample_photons];
	LOCAL vector3f_t photon_dirs [n_indir_sample_photons];


		ray_differential_t per_ray = ray[gid];
		photon_map_t photon_map;
		cl_scene_info_t scene_info;
		scene_info.light_data = light_data;
		scene_info.material_data = material_data;
		scene_info.shape_data = shape_data;
		scene_info.texture_data = texture_data;
		scene_info.integrator_data = integrator_data;
		scene_info.accelerator_data = accelerator_data;
		scene_info.primitives = primitives;
		scene_info.primitive_count = primitive_count;
		scene_info.lghts = lghts;
		scene_info.lght_count = lght_count;

		Seed seed = seeds[gid];
		spectrum_t color;
		load_photon_map(&photon_map,integrator_data);
		photon_map_li(&photon_map,&per_ray,scene_info,&seed,&color,
				ray_stack,passthrough,bsdf_stack,left_stack,close_photon_data_store,photon_buffer,photon_dirs);
		colors[gid] = color;
		seeds[gid] = seed;
		//photon_map_li(photon_map,per_ray,scene_info,seed,&color[gid]);
	}

/*	const int gid2 = 2 * gid;
	const int x = gid % width;
	const int y = gid / width;
	*/

	/* Check if we have to do something */
/*	if (y >= height)
		return;
		*/

	/*
	spectrum_t c;
	c.x = colors[i].x;
	c.y = colors[i].y;
	c.z = colors[i].z;
	pixels[y * width + x] = convert_to_rgb(&c);
	*/
}
