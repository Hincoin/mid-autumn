#ifndef _RAY_H_
#define _RAY_H_

#include "geometry.h"

typedef struct  
#ifdef CL_KERNEL
	__attribute__ ((aligned (16)))
#endif
{
	point3f_t o;
	vector3f_t d;
	float mint,maxt;
} _ray_t;


#ifndef CL_KERNEL
#include "make_aligned_type.h"
typedef make_aligned_type<_ray_t,sizeof(float)*4>::type ray_t; 
#else
typedef _ray_t ray_t;
#endif

typedef struct 
#ifdef CL_KERNEL 
	__attribute__ ((aligned (16)))
#endif
{
	point3f_t o;
	vector3f_t d;
	ray_t rx,ry;

	float mint,maxt;
	int has_differential;
}
_ray_differential_t
;

#ifndef CL_KERNEL
typedef make_aligned_type<_ray_differential_t,sizeof(float)*4>::type ray_differential_t; 
#else
typedef _ray_differential_t ray_differential_t;
#endif

#define rinit(r, a, b) { vassign((r).o, a); vassign((r).d, b);(r).mint = EPSILON;(r).maxt = FLT_MAX; }
#define rassign(a, b) { vassign((a).o, (b).o); vassign((a).d, (b).d);(a).mint = (b).mint;(a).maxt = (b).maxt; }
#define rpos(p,r,t) {p.x = r.o.x + r.d.x * t;p.y = r.o.y + r.d.y * t;p.z = r.o.z + r.d.z * t;}

#define ray_init(r, a, b) rinit(r,a,b)
#endif