#ifndef _PHOTON_INTERSECTION_DATA_H_
#define _PHOTON_INTERSECTION_DATA_H_

#include "spectrum.h"
#include "primitive.h"

typedef struct 
{
	//follow photon path through scene and record intersections
	int specular_path;
	int n_intersections;
	spectrum_t ltranmittance;
	spectrum_t alpha;
	intersection_t isect;
}
photon_intersection_data_t;


#endif