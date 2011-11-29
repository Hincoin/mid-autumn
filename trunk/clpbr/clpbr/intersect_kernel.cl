

#include "cl_config.h"
#include "geometry.h"

#include "random_number_generator.h"
#include "shape_funcs.h"
#include "primitive_funcs.h"
#include "ray.h"
#include "photon_intersection_data.h"

__kernel void photon_intersect(__global photon_intersection_data_t *intersections,GLOBAL ray_t *rays, GLOBAL unsigned int* ray_count,GLOBAL float *accelerator_data, GLOBAL float *shape_data,GLOBAL primitive_info_t *primitives,GLOBAL unsigned int *primitive_count )
{
	   const int gid = get_global_id(0);
	   if(gid < *ray_count)
	   {
			ray_t ray ;
		   	rassign(ray,rays [ gid ]) ;
			__global photon_intersection_data_t *photon_isect = intersections+gid;
 
			//photon_isect.specular_path = intersections [gid].specular_path;
			//photon_isect.n_intersections = intersections [ gid ].n_intersections;
			//photon_isect.ltranmittance = intersections[gid].ltranmittance;
			//photon_isect.alpha = intersections[gid].alpha;
			photon_isect->isect.primitive_idx = 0xffffffff;
			intersection_t isect;
			if(intersect(accelerator_data, shape_data, primitives, *primitive_count,&ray,&isect))
			{
				++photon_isect->n_intersections;
				/*todo scene_tranmittance*/
				vinit(photon_isect->ltranmittance,1.f,1.f,1.f);
				vmul(photon_isect->alpha, photon_isect->alpha, photon_isect->ltranmittance);
				photon_isect->isect = isect;
			}
			/*
			intersections[gid].specular_path = photon_isect.specular_path;
			intersections[gid].n_intersections = photon_isect.n_intersections;
			intersections[gid].ltranmittance = photon_isect.ltranmittance;
			intersections[gid].alpha = photon_isect.alpha;
			intersections[gid].isect.primitive_idx = photon_isect.isect.primitive_idx;
			intersections[gid].isect.dg = photon_isect.isect.dg;
			*/
	   }
}
