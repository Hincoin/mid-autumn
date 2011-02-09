#ifndef _CL_SCENE_H_
#define _CL_SCENE_H_

#include "config.h"
#include "primitive.h"
#include "light.h"

typedef struct  
{
GLOBAL float* light_data;
GLOBAL float* material_data;
GLOBAL float* shape_data;
GLOBAL float* texture_data;
GLOBAL float* integrator_data;
GLOBAL float* accelerator_data;

GLOBAL primitive_info_t* primitives;
GLOBAL light_info_t* lghts; 

unsigned int primitive_count;
unsigned int lght_count;
}cl_scene_info_t;

int intersectP(cl_scene_info_t scene,ray_t *r);

#endif