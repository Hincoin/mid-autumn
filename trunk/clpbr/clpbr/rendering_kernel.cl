/*
Copyright (c) 2009 David Bucciarelli (davibu@interfree.it)

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma OPENCL EXTENSION cl_amd_printf : enable

#include "cl_config.h"
#include "geometry.h"
#include "camera.h"
#include "light.h"

#include "random_number_generator.h"
#include "shape_funcs.h"
#include "path_tracing.h"
#include "photon_map.h"

//light param data
//material param data
//shape param data
//texture param data

//integrator param data
//accelerator param data
__kernel void render(
__global vector3f_t *colors, __global unsigned int *seeds,
GLOBAL float* light_data,GLOBAL float* material_data,GLOBAL float* shape_data,
GLOBAL float* texture_data,GLOBAL float* integrator_data,GLOBAL float* accelerator_data,
GLOBAL primitive_info_t* primitives,const unsigned int primitive_count,
GLOBAL light_info_t* lghts, const unsigned int lght_count,
OCL_CONSTANT_BUFFER camera_t *camera,
	const int width, const int height,
	const int currentSample,
	__global int *pixels
)
{
    const int gid = get_global_id(0);
	const int gid2 = 2 * gid;
	const int x = gid % width;
	const int y = gid / width;

	/* Check if we have to do something */
	if (y >= height)
		return;

	/* LordCRC: move seed to local store 
	unsigned int seed0 = seeds[gid2];
	unsigned int seed1 = seeds[gid2 + 1];*/
	unsigned int seed = seeds[gid];

	Seed s;
	init_rng(seed,&s);
	ray_t ray;
	GenerateCameraRay(camera, &s, width, height, x, y, &ray);

	vector3f_t r;
	PathTracing(light_data,material_data,shape_data,texture_data,integrator_data,accelerator_data,primitives,primitive_count,lghts,lght_count,
	 &ray, &s, &r);

	const int i = (height - y - 1) * width + x;
	if (currentSample == 0) {
		// Jens's patch for MacOS
		vassign(colors[i], r);
	} else {
		const float k1 = currentSample;
		const float k2 = 1.f / (currentSample + 1.f);
		colors[i].x = (colors[i].x * k1  + r.x) * k2;
		colors[i].y = (colors[i].y * k1  + r.y) * k2;
		colors[i].z = (colors[i].z * k1  + r.z) * k2;
	}

	spectrum_t c;
	c.x = colors[i].x;
	c.y = colors[i].y;
	c.z = colors[i].z;
	pixels[y * width + x] = convert_to_rgb(&c);

	seeds[gid] = random_uint(&s);
}
