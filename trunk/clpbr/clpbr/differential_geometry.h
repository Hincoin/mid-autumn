#ifndef _DIFFERENTIAL_GEOMETRY_H_
#define _DIFFERENTIAL_GEOMETRY_H_



#include "geometry.h"
#include "matrix4x4.h"
#include "ray.h"

typedef struct {
	
	point3f_t p;
	normal3f_t nn;
	vector3f_t dpdu, dpdv;
	vector3f_t dndu, dndv;

	vector3f_t dpdx, dpdy;
	float dudx, dvdx, dudy, dvdy;
	float u, v;
	unsigned shape_idx;
}differential_geometry_t;

#define differential_geometry_init(_dg,_p,_dpdu,_dpdv,_dndu,_dndv,_uu,_vv,shape)\
{\
	vassign((_dg).p,(_p));\
	vassign((_dg).dpdu,(_dpdu));\
	vassign((_dg).dpdv,(_dpdv));\
	vassign((_dg).dndu,(_dndu));\
	vassign((_dg).dndv,(_dndv));\
	(_dg).u = (_uu);\
	(_dg).v = (_vv);\
	vxcross((_dg).nn, (_dpdu), (_dpdv));\
	vnorm((_dg).nn);\
	if((shape)->reverse_orientation /*^ transform_swaphandedness()*/)\
		vsmul((_dg).nn,-1.f,(_dg).nn);\
	(_dg).dudx = (_dg).dvdx = (_dg).dudy = (_dg).dvdy = 0;\
}\

//todo
#define compute_differential_geometry(_dg,r)\
{\
	(_dg).dudx = (_dg).dvdx = 0.f;\
	(_dg).dudy = (_dg).dvdy = 0.f;\
	vclr((_dg).dpdx) ;\
	vclr((_dg).dpdy);\
}\

/*
if (ray.hasDifferentials) {
	// Estimate screen-space change in \pt and $(u,v)$
	// Compute auxiliary intersection points with plane
	float d = -Dot(nn, Vector(p.x, p.y, p.z));
	Vector rxv(ray.rx.o.x, ray.rx.o.y, ray.rx.o.z);
	float tx = -(Dot(nn, rxv) + d) / Dot(nn, ray.rx.d);
	Point px = ray.rx.o + tx * ray.rx.d;
	Vector ryv(ray.ry.o.x, ray.ry.o.y, ray.ry.o.z);
	float ty = -(Dot(nn, ryv) + d) / Dot(nn, ray.ry.d);
	Point py = ray.ry.o + ty * ray.ry.d;
	dpdx = px - p;
	dpdy = py - p;
	// Compute $(u,v)$ offsets at auxiliary points
	// Initialize _A_, _Bx_, and _By_ matrices for offset computation
	float A[2][2], Bx[2], By[2], x[2];
	int axes[2];
	if (fabsf(nn.x) > fabsf(nn.y) && fabsf(nn.x) > fabsf(nn.z)) {
		axes[0] = 1; axes[1] = 2;
	}
	else if (fabsf(nn.y) > fabsf(nn.z)) {
		axes[0] = 0; axes[1] = 2;
	}
	else {
		axes[0] = 0; axes[1] = 1;
	}
	// Initialize matrices for chosen projection plane
	A[0][0] = dpdu[axes[0]];
	A[0][1] = dpdv[axes[0]];
	A[1][0] = dpdu[axes[1]];
	A[1][1] = dpdv[axes[1]];
	Bx[0] = px[axes[0]] - p[axes[0]];
	Bx[1] = px[axes[1]] - p[axes[1]];
	By[0] = py[axes[0]] - p[axes[0]];
	By[1] = py[axes[1]] - p[axes[1]];
	if (SolveLinearSystem2x2(A, Bx, x)) {
		dudx = x[0]; dvdx = x[1];
	}
	else  {
		dudx = 1.; dvdx = 0.;
	}
	if (SolveLinearSystem2x2(A, By, x)) {
		dudy = x[0]; dvdy = x[1];
	}
	else {
		dudy = 0.; dvdy = 1.;
	}
}
else {
	dudx = dvdx = 0.;
	dudy = dvdy = 0.;
	dpdx = dpdy = Vector(0,0,0);
}
*/



#endif