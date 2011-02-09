#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

//vector_t point_t normal_t 
//ray_differential_t bbox_t 
//opt:function : spherical direction, spherical theta,phi;


typedef struct 
{
	float x,y,z;
}vector3f_t;

typedef struct 
{
	float x,y,z;
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

#ifndef INV_PI
#define INV_PI 0.3183099f //0.318309886183790671538
#endif

#define RAD_PER_DEGREE 0.0174533f //FLOAT_PI / 180.f
#define radians(_deg) (_deg * RAD_PER_DEGREE)

#endif