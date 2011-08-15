#ifndef _RAY_HIT_POINT_H_
#define _RAY_HIT_POINT_H_

#include "geometry.h"
#include "reflection.h"
#include "spectrum.h"

typedef enum{hp_constant_color,hp_surface}hit_point_type;

struct ray_hit_point_t
{
	/*spectrum_t flux,accum_flux;
	spectrum_t radiance,accum_radiance;
	float r2;
	unsigned photon_count,accum_photon_count;
	unsigned pixel_sample_index,hit_count;
	unsigned constant_hit_count;
	unsigned surface_hit_count;*/

	//ray tracing pass
	unsigned index;
	hit_point_type type;
	spectrum_t throughput;
	point3f_t pos;
	vector3f_t wo;
	normal3f_t normal;
	bsdf_t bsdf;
};



#endif