#ifndef _PHOTON_MAP_H_
#define _PHOTON_MAP_H_

//
#include "kdtree.h"
#include "spectrum.h"
#include "geometry.h"
#include "sampling.h"
#include "random_number_generator.h"
#include "primitive_funcs.h"

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

typedef struct{
	const photon_t* photon;
	float distance_squared;
}
close_photon_t;

INLINE void close_photon_init(close_photon_t* close_photon,const photon_t* photon,float dist_sqr)
{
	close_photon->photon = photon;
	close_photon->distance_squared = dist_sqr;
}
INLINE bool close_photon_less_than(close_photon_t a,close_photon_t b)
{
	return a.distance_squared == b.distance_squared? (a.photon < b.photon) :
		a.distance_squared < b.distance_squared;
}
typedef struct  
{
	const point3f_t* p;
	close_photon_t *photons;
	unsigned n_lookup;
	unsigned found_photons;
}photon_process_data_t;

INLINE void photon_process_data_init(photon_process_data_t* photon_process_data,
						 unsigned mp,
						 const point3f_t* p)
{
	photon_process_data->p = p;
	photon_process_data->photons = 0;
	photon_process_data->n_lookup = mp;
	photon_process_data->found_photons = 0;
}
INLINE void photon_process(photon_process_data_t* data,const photon_t* photon,
					float dist2,float* max_dist_sqred)
{
	if (data->found_photons < data->n_lookup) {
		// Add photon to unordered array of photons
		close_photon_init(&data->photons[data->found_photons++] ,photon, dist2);
		if (data->found_photons == data->n_lookup) {
			c_make_heap(close_photon_t,&data->photons[0], &data->photons[data->n_lookup],close_photon_less_than);
			*max_dist_sqred = data->photons[0].distance_squared;
		}
	}
	else {
		// Remove most distant photon from heap and add new photon
		close_photon_t *begin = data->photons;
		close_photon_t *end = data->photons + data->n_lookup;
		c_pop_heap(close_photon_t,begin, end,close_photon_less_than);
		close_photon_init(&data->photons[data->n_lookup-1] ,photon, dist2);
		c_push_heap(close_photon_t,begin, end,close_photon_less_than);
		*max_dist_sqred = data->photons[0].distance_squared;
	}
}
typedef kd_tree_t(photon_t) photon_kd_tree_t;
typedef kd_tree_t(radiance_photon_t) radiance_photon_kd_tree_t;

typedef struct {
	// ExPhotonIntegrator Private Data
    unsigned n_caustic_photons, n_indirect_photons;
	unsigned  n_lookup;
	int specular_depth;
	int max_specular_depth;
	float max_dist_squared, rr_threshold;
	float cos_gather_angle;
	int gather_samples;
	// Declare sample parameters for light source sampling
	int n_caustic_paths, n_indirect_paths;
	photon_kd_tree_t caustic_map;
	photon_kd_tree_t indirect_map;
	radiance_photon_kd_tree_t radiance_map;
    bool final_gather;
}
photon_map_t;

#define MAX_CLOSE_PHOTON_LOOKUP 64

INLINE void photon_map_lphoton(photon_map_t* photon_map,
							   photon_kd_tree_t *map,
							   int n_paths,
							   int n_lookup,
							   bsdf_t *bsdf,
							   const intersection_t* isect,
							   const vector3f_t *wo,
							   float max_dist_sqr,
							   cl_scene_info_t scene_info,
							   Seed *seed)
{

}
INLINE void photon_map_li(photon_map_t* photon_map
						  )
{
}
#ifndef CL_KERNEL

void photon_map_init(photon_map_t* photon_map,GLOBAL float* light_data,GLOBAL float* material_data,GLOBAL float* shape_data,
					 GLOBAL float* texture_data,GLOBAL float* integrator_data,GLOBAL float* accelerator_data,
					 GLOBAL primitive_info_t* primitives,const unsigned int primitive_count,
					 GLOBAL light_info_t* lghts, const unsigned int lght_count,
					 Seed* seed);
#endif
#endif