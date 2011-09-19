#ifndef _RAY_H_
#define _RAY_H_

#include "geometry.h"

typedef struct  
{
	point3f_t o;
	vector3f_t d;
	float mint,maxt;
} ray_t;

struct ray_differential_t:public ray_t
{
	char has_differential;
	ray_t rx,ry;
	unsigned ray_id;
};

#define rinit(r, a, b) { vassign((r).o, a); vassign((r).d, b);(r).mint = EPSILON;(r).maxt = FLT_MAX; }
#define rassign(a, b) { vassign((a).o, (b).o); vassign((a).d, (b).d);(a).mint = (b).mint;(a).maxt = (b).maxt; }
#define rpos(p,r,t) {p.x = r.o.x + r.d.x * t;p.y = r.o.y + r.d.y * t;p.z = r.o.z + r.d.z * t;}

#define ray_init(r, a, b) rinit(r,a,b)
#endif