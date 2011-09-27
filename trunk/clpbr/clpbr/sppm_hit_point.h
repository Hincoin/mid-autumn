#ifndef _SPPM_HIT_POINT_H_
#define _SPPM_HIT_POINT_H_

#include "sppm.h"
//per pass information
struct accum_hit_point_t 
{
	explicit accum_hit_point_t(float radius2 = -1.f):accum_photon_count(0),r2(radius2){vclr(accum_flux);}
	spectrum_t accum_flux;
	float r2;
	unsigned accum_photon_count;
};

struct final_hit_point_t 
{
	final_hit_point_t():photon_count(0),hit_count(0),constant_hit_count(0),
		surface_hit_count(0),r2(-1){
			vclr(flux);
			vclr(radiance);
			vclr(accum_radiance);
	}
	unsigned photon_count;
	unsigned hit_count;
	unsigned constant_hit_count;
	unsigned surface_hit_count;
	spectrum_t flux,radiance;
	spectrum_t accum_radiance;
	float r2;
};

#endif