#ifndef _PHOTON_MAP_H_
#define _PHOTON_MAP_H_

//
#include "kdtree.h"
#include "spectrum.h"
#include "geometry.h"

typedef struct{
	point3f_t p;
	spectrum_t alpha;
	vector3f_t wi;
} photon_t;
INLINE void photon_init(photon_t* photon,const point3f_t* p,const spectrum_t* wt,const vector3f_t* w)
{
	photon->p = *p;
	photon->alpha = *wt;
	photon->wi = *w;
}

typedef struct {
	point3f_t p;
	normal3f_t n;
	spectrum_t lo;
}radiance_photon_t ;

INLINE void radiance_photon_init(radiance_photon_t* photon,const point3f_t* p,const normal3f_t* n)
{
	photon->p = *p;
	photon->n = *n;
	vclr(photon->lo);
}

typedef kd_tree_t(photon_t) photon_kd_tree_t;
typedef kd_tree_t(radiance_photon_t) radiance_photon_kd_tree_t;

void photon_map_li()
{
}
#endif