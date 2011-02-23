#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

//vector_t point_t normal_t 
//ray_differential_t bbox_t 
//opt:function : spherical direction, spherical theta,phi;

#include "config.h"


typedef struct 
{
	float x,y,z;
}vector3f_t;

typedef struct 
{
	float x,y,z;
#ifndef CL_KERNEL
	float& operator[](int i)
	{
		if(i == 0)return x;
		if(i == 1)return y;
		if(i == 2)return z;
	}
	float operator[](int i)const
	{
		if(i == 0)return x;
		if(i == 1)return y;
		if(i == 2)return z;
	}
#endif
}point3f_t;

typedef struct 
{
	float x,y,z;
}normal3f_t;


#define vinit(v, a, b, c) { (v).x = a; (v).y = b; (v).z = c; }
#define vassign(a, b) vinit(a, (b).x, (b).y, (b).z)
#define vclr(v) vinit(v, 0.f, 0.f, 0.f)
#define vadd(v, a, b) vinit(v, (a).x + (b).x, (a).y + (b).y, (a).z + (b).z)
#define vsub(v, a, b) vinit(v, (a).x - (b).x, (a).y - (b).y, (a).z - (b).z)
#define vsadd(v, a, b) { float k = (a); vinit(v, (b).x + k, (b).y + k, (b).z + k) }
#define vssub(v, a, b) { float k = (a); vinit(v, (b).x - k, (b).y - k, (b).z - k) }
#define vmul(v, a, b) vinit(v, (a).x * (b).x, (a).y * (b).y, (a).z * (b).z)
#define vsmul(v, a, b) { float _k = (a); vinit(v, _k * (b).x, _k * (b).y, _k * (b).z) }
#define vdot(a, b) ((a).x * (b).x + (a).y * (b).y + (a).z * (b).z)

#define vnorm(v) { float _l = 1.f / sqrt(vdot(v, v)); vsmul(v, _l, v); }
#define vxcross(v, a, b) vinit(v, (a).y * (b).z - (a).z * (b).y, (a).z * (b).x - (a).x * (b).z, (a).x * (b).y - (a).y * (b).x)
#define vfilter(v) ((v).x > (v).y && (v).x > (v).z ? (v).x : (v).y > (v).z ? (v).y : (v).z)
#define viszero(v) (((v).x == 0.f) && ((v).x == 0.f) && ((v).z == 0.f))
#define vdiv(v,a,b) {(v).x = (a).x / (b).x;(v).y = (a).y/(b).y;(v).z = (a).z/(b).z;}
#define vneg(v0,v) {(v0).x = -(v).x;(v0).y = -(v).y;(v0).z = -(v).z;}

#ifndef CL_KERNEL
#define clamp(x, a, b) ((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))
#define max(x, y) ( (x) > (y) ? (x) : (y))
#define min(x, y) ( (x) < (y) ? (x) : (y))
#define sign(x) ((x) > 0 ? 1 : -1)
#endif

#define to_int(x) ((int)(pow(clamp(x, 0.f, 1.f), 1.f / 2.2f) * 255.f + .5f))



#ifndef EPSILON
#define EPSILON 1e-3f //1.192092896e-07F 
#endif
#ifndef FLOAT_PI
#define FLOAT_PI 3.1415626f
#endif

#ifndef FLT_MAX
#define FLT_MAX 3.402823466e+38F
#endif

#ifndef INV_PI
#define INV_PI 0.3183099f //0.318309886183790671538
#endif

#define RAD_PER_DEGREE 0.0174533f //FLOAT_PI / 180.f
#define radians(_deg) (_deg * RAD_PER_DEGREE)



typedef struct  
{
	point3f_t pmin,pmax;
}bbox_t;

INLINE void bbox_init(bbox_t* bbox)
{
	vinit(bbox->pmin,FLT_MAX,FLT_MAX,FLT_MAX);
	vinit(bbox->pmax,-FLT_MAX,-FLT_MAX,-FLT_MAX);
}
INLINE void bbox_union_with_point(bbox_t* bbox,const point3f_t* p)
{
	bbox->pmin.x = min(bbox->pmin.x, p->x);
    bbox->pmin.y = min(bbox->pmin.y, p->y);
    bbox->pmin.z = min(bbox->pmin.z, p->z);
    bbox->pmax.x = max(bbox->pmax.x, p->x);
    bbox->pmax.y = max(bbox->pmax.y, p->y);
    bbox->pmax.z = max(bbox->pmax.z, p->z);
}
//return which axis is has max distance
INLINE int bbox_max_extent(const bbox_t* bbox)
{
	vector3f_t diag;
	vsub(diag,bbox->pmax,bbox->pmin);
	if (diag.x > diag.y && diag.x > diag.z)
		return 0;
	else if (diag.y > diag.z)
		return 1;
	else
		return 2;
}

INLINE void coordinate_system(const vector3f_t *v1, vector3f_t *v2, vector3f_t *v3) {
	if (fabs(v1->x) > fabs(v1->y)) {
		float invLen = 1.f / sqrt(v1->x*v1->x + v1->z*v1->z);
		vinit(*v2,-v1->z * invLen,0.f,v1->x * invLen);
	}
	else {
		float invLen = 1.f / sqrt(v1->y*v1->y + v1->z*v1->z);
		vinit(*v2,0.f,v1->z * invLen, -v1->y * invLen);
	}
	vxcross(*v3,*v1, *v2);
}


#endif