
#pragma OPENCL EXTENSION cl_amd_printf : enable

#include "cl_config.h"
#include "geometry.h"
#include "light.h"

#include "random_number_generator.h"
#include "shape_funcs.h"
#include "photon_map.h"

//light param data
//material param data
//shape param data
//texture param data

__kernel void render(
__global spectrum_t *colors, __global unsigned int *seeds,
GLOBAL float* light_data,GLOBAL float* material_data,GLOBAL float* shape_data,
GLOBAL float* texture_data,GLOBAL float* integrator_data,GLOBAL float* accelerator_data,
GLOBAL primitive_info_t* primitives,const unsigned int primitive_count,
GLOBAL light_info_t* lghts, const unsigned int lght_count
)
{
    const int gid = get_global_id(0);
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
