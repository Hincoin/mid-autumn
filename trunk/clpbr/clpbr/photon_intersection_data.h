#ifndef _PHOTON_INTERSECTION_DATA_H_
#define _PHOTON_INTERSECTION_DATA_H_

#include "spectrum.h"
#include "primitive.h"

#include "photon_map.h"
typedef struct 
#ifdef CL_KERNEL
	__attribute__ ((aligned (16)))
#endif
{
	//follow photon path through scene and record intersections
	//int specular_path;
	//int has_non_specular;
	//int n_intersections;
	//int continue_trace;
	intersection_t isect;
	spectrum_t alpha;
	Seed seed;
	int packed_data;//specular_path:1bit,has_non_specular:1bit,continue_trace:1bit,n_intersections:29 bit
}
_photon_intersection_data_t;

typedef struct
#ifdef CL_KERNEL
	__attribute__ ((aligned (16)))
#endif
{
	photon_t photon;
	//final gather
	radiance_photon_t radiance_photon;
	spectrum_t rho_r,rho_t;
}_generated_photon_t;

#ifndef CL_KERNEL
#include "make_aligned_type.h"
typedef make_aligned_type<_photon_intersection_data_t,sizeof(float)*4>::type photon_intersection_data_t;
static_assert(sizeof(photon_intersection_data_t)%(4*sizeof(float))==0,"photon_intersectoin_data_t's size not aligned!");

typedef make_aligned_type<_generated_photon_t,sizeof(float)*4>::type generated_photon_t;
static_assert(sizeof(generated_photon_t)%(4*sizeof(float))==0,"generated_photon_t 's size not aligned!");

#else
typedef _photon_intersection_data_t photon_intersection_data_t;
typedef _generated_photon_t generated_photon_t;
#endif


INLINE bool photon_intersection_data_is_specular_path(const photon_intersection_data_t* data)
{
	return (data->packed_data & 0x80000000) != 0;
}

INLINE void photon_intersection_data_set_specular_path(photon_intersection_data_t* data,unsigned int is_specular)
{
	data->packed_data = ((is_specular << 31) & 0x80000000) | (data->packed_data & ~0x80000000);
}
INLINE bool photon_intersection_data_has_non_specular(const photon_intersection_data_t* data)
{
	return (data->packed_data & 0x40000000) != 0;
}
INLINE void photon_intersection_data_set_non_specular(photon_intersection_data_t* data, unsigned int has_non_specular)
{
	data->packed_data = ((has_non_specular << 30) & 0x40000000) | (data->packed_data & ~0x40000000);
}
INLINE bool photon_intersection_data_continue_trace(const photon_intersection_data_t* data)
{
	return (data->packed_data & 0x20000000) != 0;
}
INLINE void photon_intersection_data_set_continue_trace(photon_intersection_data_t* data,unsigned int continue_trace)
{
	data->packed_data = ((continue_trace << 29) & 0x20000000) | (data->packed_data & ~0x20000000);
}
INLINE int photon_intersection_data_n_intersections(const photon_intersection_data_t* data)
{
	return (data->packed_data & 0xffff);
}

INLINE void photon_intersection_data_set_n_intersections(photon_intersection_data_t* data, int n_intersections)
{
	data->packed_data = n_intersections | (data->packed_data & 0xffff0000);
}
#endif