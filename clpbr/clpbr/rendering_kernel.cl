
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
GLOBAL primitive_info_t* primitives,GLOBAL unsigned int *primitive_count,
GLOBAL light_info_t* lghts, GLOBAL unsigned int *lght_count,GLOBAL ray_differential_t *ray,GLOBAL unsigned int *number_work_items
)
{
    const int gid = get_global_id(0);
	if(gid < *number_work_items)
	{
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
		scene_info.primitive_count = *primitive_count;
		scene_info.lghts = lghts;
		scene_info.lght_count = *lght_count;

		GLOBAL kd_node_t* nodes = (GLOBAL kd_node_t*)(integrator_data);

		Seed seed = seeds[gid];
		spectrum_t color;
		load_photon_map(&photon_map,integrator_data);
		photon_map_li(&photon_map,&per_ray,scene_info,&seed,&color);
		colors[gid] = color;
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
