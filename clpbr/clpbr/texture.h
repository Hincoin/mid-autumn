#ifndef _TEXTURE_H_
#define _TEXTURE_H_
#include "spectrum.h"

typedef struct {
	float value;
}float_constant_texture_t;
typedef struct {
	spectrum_t value;
}color_constant_texture_t;

typedef struct
{
	int texture_type;
	unsigned memory_start;
}
float_texture_info_t;

typedef struct
{
	int texture_type;
	unsigned memory_start;
}
color_texture_info_t;

#include "cl_scene.h"
#include "shape.h"

typedef enum{FLOAT_CONSTANT=0,COLOR_CONSTANT}TextureType;
//interfaces

void color_texture_evaluate(color_texture_info_t* tex,cl_scene_info_t* scene_data,
							differential_geometry_t* dg,spectrum_t* c);
void float_texture_evaluate(float_texture_info_t* tex,cl_scene_info_t* scene_data,
							differential_geometry_t* dg,float* c);


INLINE void color_texture_evaluate(color_texture_info_t* tex,cl_scene_info_t* scene_data,
							differential_geometry_t* dg,spectrum_t* c)
{
	switch(tex->texture_type )
	{
	case COLOR_CONSTANT:
		{
			GLOBAL float* mem = scene_data->texture_data + tex->memory_start;
			c->x = mem[0];
			c->y = mem[1];
			c->z = mem[2];
		}
		break;
	default:
		break;
	}
}
INLINE void float_texture_evaluate(float_texture_info_t* tex,cl_scene_info_t* scene_data,
							differential_geometry_t* dg,float* c)
{
	switch(tex->texture_type)
	{
	case FLOAT_CONSTANT:
		{
			GLOBAL float* mem = scene_data->texture_data + tex->memory_start;
			*c = mem[0];
		}
		break;
	default:
		break;
	}
		
}
#endif