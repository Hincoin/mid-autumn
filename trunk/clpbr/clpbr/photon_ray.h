#ifndef _PHOTON_RAY_H_
#define _PHOTON_RAY_H_
#include "ray.h"
#include "spectrum.h"

struct photon_ray_t:ray_t
{
    photon_ray_t(){vclr(flux);}
	mutable spectrum_t flux;
};
struct photon_ray_hit_point_t{
	vector3f_t n;
	point3f_t pos;
	vector3f_t wo;
};
#endif
