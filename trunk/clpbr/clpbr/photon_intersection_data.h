#ifndef _PHOTON_INTERSECTION_DATA_H_
#define _PHOTON_INTERSECTION_DATA_H_

#include "spectrum.h"
#include "primitive.h"

#include "photon_map.h"
typedef struct 
{
	//follow photon path through scene and record intersections
	int specular_path;
	int has_non_specular;
	int n_intersections;
	int continue_trace;
	spectrum_t alpha;
	photon_t photon;
}
photon_intersection_data_t;


#endif