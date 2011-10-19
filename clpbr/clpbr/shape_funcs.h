#ifndef _SHAPE_FUNC_H_
#define _SHAPE_FUNC_H_

#include "shape.h"
#include "quadratic_math.h"

//
//float SphereIntersect(
//#ifdef CL_KERNEL
//					  OCL_CONSTANT_BUFFER
//#endif
//					  const sphere_t*s,
//					  const ray_t *r) { /* returns distance, 0 if nohit */
//						  vector3f_t op; /* Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0 */
//						  vsub(op, s->p, r->o);
//
//						  float b = vdot(op, r->d);
//						  float det = b * b - vdot(op, op) + s->rad * s->rad;
//						  if (det < 0.f)
//							  return 0.f;
//						  else
//							  det = sqrt(det);
//
//						  float t = b - det;
//						  if (t >  EPSILON)
//							  return t;
//						  else {
//							  t = b + det;
//
//							  if (t >  EPSILON)
//								  return t;
//							  else
//								  return 0.f;
//						  }
//}
//
//static int Intersect(
//#ifdef CL_KERNEL
//					 OCL_CONSTANT_BUFFER
//#endif
//					 const sphere_t *spheres,
//					 const unsigned int sphereCount,
//					 const ray_t *r,
//					 float *t,
//					 unsigned int *id) {
//						 float inf = (*t) = 1e20f;
//
//						 unsigned int i = sphereCount;
//						 for (; i--;) {
//							 const float d = SphereIntersect(&spheres[i], r);
//							 if ((d != 0.f) && (d < *t)) {
//								 *t = d;
//								 *id = i;
//							 }
//						 }
//
//						 return (*t < inf);
//}
//
//static int IntersectP(
//#ifdef CL_KERNEL
//					  OCL_CONSTANT_BUFFER
//#endif
//					  const sphere_t *spheres,
//					  const unsigned int sphereCount,
//					  const ray_t *r,
//					  const float maxt) {
//						  unsigned int i = sphereCount;
//						  for (; i--;) {
//							  const float d = SphereIntersect(&spheres[i], r);
//							  if ((d != 0.f) && (d < maxt))
//								  return 1;
//						  }
//
//						  return 0;
//}



#include "shape.h"
#include "mc.h"
INLINE void sphere_init(sphere_t* sphere,transform_t* tf,
						float rad,float z0,float z1,float pm,unsigned ro)
{
	transform_assign(sphere->o2w,*tf);
	sphere->rad = rad;
	sphere->zmin = clamp(min(z0,z1),-rad,rad);
	sphere->zmax = clamp(max(z0,z1),-rad,rad);
	sphere->thetaMin = acos(z0/rad);
	sphere->thetaMax = acos(z1/rad);
	sphere->phiMax = radians(clamp(pm,0.f,360.f));

	sphere->reverse_orientation = ro;
}
//todo
INLINE void store_sphere(float** mem,sphere_t* s)
{
}
INLINE void load_sphere(GLOBAL float* mem, sphere_t *ret)
{
	transform_load_from_array(ret->o2w,mem);
	unsigned base = sizeof(ret->o2w)/sizeof(float);
	ret->rad = mem[base];
	ret->phiMax = mem[base+1];
	ret->zmin = mem[base+2];
	ret->zmax = mem[base+3];
	ret->thetaMin = mem[base+4];
	ret->thetaMax = mem[base+5];
	vinit(ret->p,mem[base+6],mem[base+7],mem[base+8]);
	ret->reverse_orientation = as_uint(mem[base+9]);
}



//
INLINE int intersect_sphereP(GLOBAL float* shape_data,const unsigned int memory_start,
					  ray_t *r
					  )
{
	sphere_t sphere ;
	load_sphere(shape_data+memory_start,&sphere);

	float phi;
	point3f_t phit;
	// Transform _Ray_ to object space
	ray_t ray;
	//transform_construct_from_matrix(w2o, sphere.o2w.inv_m);
	inverse_transform_ray(ray,sphere.o2w,*r);
	//transform_point(ray.o,sphere.o2w,r->o);
	// Compute quadratic sphere coefficients
	float A = ray.d.x*ray.d.x + ray.d.y*ray.d.y +
		ray.d.z*ray.d.z;
	float B = 2 * (ray.d.x*ray.o.x + ray.d.y*ray.o.y +
		ray.d.z*ray.o.z);
	float C = ray.o.x*ray.o.x + ray.o.y*ray.o.y +
		ray.o.z*ray.o.z - sphere.rad*sphere.rad;
	// Solve quadratic equation for _t_ values
	float t0, t1;
	if (!Quadratic(A, B, C, &t0, &t1))
	{
		return 0;
	}
	// Compute intersection distance along ray
	if (t0 > ray.maxt || t1 < ray.mint)
	{
		return 0;
	}
	float thit = t0;
	if (t0 < ray.mint) {
		thit = t1;
		if (thit > ray.maxt) {return 0;}
	}
	// Compute sphere hit position and $\phi$
	rpos(phit,ray,thit);
	phi = atan2(phit.y, phit.x);
	if (phi < 0.) phi += 2.f*FLOAT_PI;
	// Test sphere intersection against clipping parameters
	if (phit.z < sphere.zmin || phit.z > sphere.zmax || phi > sphere.phiMax) {
		if (thit == t1) {return 0;}
		if (t1 > ray.maxt) {return 0;}
		thit = t1;
		// Compute sphere hit position and $\phi$
		rpos(phit,ray,thit);
		phi = atan2(phit.y, phit.x);
		if (phi < 0.) phi += 2.f*FLOAT_PI;
		if (phit.z < sphere.zmin || phit.z > sphere.zmax || phi > sphere.phiMax)
			return 0;
	}
	return 1;	
}
INLINE int intersect_sphere(GLOBAL float* shape_data,const unsigned int memory_start,
					 ray_t *r,
					 float *tHit,
					 differential_geometry_t *dg,
					 float *epsilon)
{
	sphere_t sphere ;
	load_sphere(shape_data+memory_start,&sphere);

	float phi;
	point3f_t phit;
	// Transform _Ray_ to object space
	ray_t ray;
	inverse_transform_ray(ray,sphere.o2w,*r);
	//transform_point(ray.o,sphere.o2w,r->o);
	// Compute quadratic sphere coefficients
	float A = ray.d.x*ray.d.x + ray.d.y*ray.d.y +
		ray.d.z*ray.d.z;
	float B = 2 * (ray.d.x*ray.o.x + ray.d.y*ray.o.y +
		ray.d.z*ray.o.z);
	float C = ray.o.x*ray.o.x + ray.o.y*ray.o.y +
		(ray.o.z-sphere.rad)*(ray.o.z+sphere.rad);
		//ray.o.z*ray.o.z - sphere.rad*sphere.rad;
	// Solve quadratic equation for _t_ values
	float t0, t1;
	if (!Quadratic(A, B, C, &t0, &t1))
	{
		return 0;
	}
	// Compute intersection distance along ray
	if (t0 > ray.maxt || t1 < ray.mint)
	{
		return 0;
	}
	float thit = t0;
	if (t0 < ray.mint) {
		thit = t1;
		if (thit > ray.maxt) {return 0;}
	}
	// Compute sphere hit position and $\phi$
	rpos(phit,ray,thit);
	phi = atan2(phit.y, phit.x);
	if (phi < 0.) phi += 2.f*FLOAT_PI;
	// Test sphere intersection against clipping parameters
	if (sphere.zmin > - sphere.rad && phit.z < sphere.zmin 
		|| 
		sphere.zmax < sphere.rad && phit.z > sphere.zmax 
		|| phi > sphere.phiMax) {
		if (thit == t1) {return 0;}
		if (t1 > ray.maxt) {return 0;}
		thit = t1;
		// Compute sphere hit position and $\phi$
		rpos(phit,ray,thit);
		phi = atan2(phit.y, phit.x);
		if (phi < 0.) phi += 2.f*FLOAT_PI;
		if (sphere.zmin > -sphere.rad && phit.z < sphere.zmin 
			|| sphere.zmax < sphere.rad && phit.z > sphere.zmax || phi > sphere.phiMax)
			return 0;
	}
	// Find parametric representation of sphere hit
	float u = phi / sphere.phiMax;
	float theta = acos(clamp(phit.z / sphere.rad,-1.f,1.f));
	float v = (theta - sphere.thetaMin) / (sphere.thetaMax - sphere.thetaMin);
	// Compute sphere \dpdu and \dpdv
	float cosphi, sinphi;
	vector3f_t dpdu, dpdv;
	float zradius = sqrt(phit.x*phit.x + phit.y*phit.y);
	vector3f_t tmp;
	if (zradius == 0)
	{
		// Handle hit at degenerate parameterization point
		cosphi = 0;
		sinphi = 1;
		vinit(tmp,phit.z * cosphi,phit.z * sinphi,-sphere.rad * sin(theta));
		vsmul(dpdv,sphere.thetaMax-sphere.thetaMin,tmp) ;
		vector3f_t norm ;
		vassign(norm,phit);
		vxcross(dpdu,dpdv,norm);
	}
	else
	{
		float invzradius = 1.f / zradius;
		cosphi = phit.x * invzradius;
		sinphi = phit.y * invzradius;
		vinit(dpdu,-sphere.phiMax * phit.y, sphere.phiMax * phit.x, 0);
		vinit(tmp,phit.z * cosphi, phit.z * sinphi, - sphere.rad * sin(theta));
		vsmul(dpdv,sphere.thetaMax - sphere.thetaMin,tmp);
	}
	// Compute sphere \dndu and \dndv
	vinit(tmp,phit.x,phit.y,0);
	vector3f_t d2Pduu;
	vsmul(d2Pduu,-sphere.phiMax*sphere.phiMax,tmp);

	vinit(tmp,-sinphi, cosphi,0.f);
	vector3f_t d2Pduv;
	vsmul(d2Pduv,(sphere.thetaMax - sphere.thetaMin) * phit.z * sphere.phiMax, tmp);

	vector3f_t d2Pdvv;
	vinit(tmp,phit.x,phit.y,phit.z)	;
	vsmul(d2Pdvv,( -(sphere.thetaMax - sphere.thetaMin) * (sphere.thetaMax - sphere.thetaMin) ),tmp);
	// Compute coefficients for fundamental forms
	float E = vdot(dpdu, dpdu);
	float F = vdot(dpdu, dpdv);
	float G = vdot(dpdv, dpdv);

	vector3f_t N;
	vxcross(tmp,dpdu,dpdv);
	vnorm(tmp);
	vassign(N,tmp);

	float e = vdot(N, d2Pduu);
	float f = vdot(N, d2Pduv);
	float g = vdot(N, d2Pdvv);
	// Compute \dndu and \dndv from fundamental form coefficients
	float invEGF2 = 1.f / (E*G - F*F);
	vector3f_t dndu;
	vsmul(dndu,(f*F - e*G) * invEGF2, dpdu);
	vsmul(tmp,(e*F - f*E) * invEGF2 , dpdv)
		vadd(dndu,dndu,tmp)

		vector3f_t dndv;
	vsmul(dndv,(g*F - f*G) * invEGF2, dpdu);
	vsmul(tmp,(f*F - g*E) * invEGF2 , dpdv)
		vadd(dndv,dndv,tmp);

	// Initialize _DifferentialGeometry_ from parametric information
	transform_point(dg->p,sphere.o2w,phit);
	transform_vector(dg->dpdu,sphere.o2w,dpdu);
	transform_vector(dg->dpdv,sphere.o2w,dpdv);
	transform_vector(dg->dndu,sphere.o2w,dndu);
	transform_vector(dg->dndv,sphere.o2w,dndv);
	differential_geometry_init(*dg,dg->p,dg->dpdu,dg->dpdv,dg->dndu,dg->dndv,u,v,&sphere)
		// Update _tHit_ for quadric intersection
		*tHit = thit;
	*epsilon = 5e-4f * *tHit;
	return 1;
}
#include "cl_scene.h"
INLINE float shape_area(shape_info_t shape_info,cl_scene_info_t scene_info)
{
	switch(shape_info.shape_type)
	{
	case 0:
		{
			sphere_t s;
			load_sphere(scene_info.shape_data + shape_info.memory_start
				,&s);
			return FLOAT_PI * s.rad * s.rad * 4;
		}
		break;
	default:
		return 0;
	}
}


INLINE void sphere_shape_sample(sphere_t* s,cl_scene_info_t scene_info,float u0,float u1,normal3f_t *ns,point3f_t *pr)
{
	//sample shape
	vector3f_t new_v;
	UniformSampleSphere(u0,u1,&new_v);
	vsmul(new_v,s->rad,new_v);
	transform_normal(*ns,s->o2w,new_v);
	vnorm(*ns);
	if(s->reverse_orientation)vsmul(*ns,-1.f,*ns);
	transform_point(*pr,s->o2w,new_v);
}
INLINE void shape_sample_on_shape(shape_info_t* shape_info,cl_scene_info_t scene_info,float u0,float u1,normal3f_t *ns,point3f_t *pr)
{
	if (shape_info->shape_type == 0)
	{
		sphere_t s;
		load_sphere(scene_info.shape_data + shape_info->memory_start
			,&s);
		sphere_shape_sample(&s,scene_info,u0,u1,ns,pr);
	}
}
INLINE void shape_sample(shape_info_t* shape_info,cl_scene_info_t scene_info,const point3f_t* p,float u0,float u1,normal3f_t *ns,point3f_t *pr)
{
	//
	if (shape_info->shape_type == 0)
	{
		sphere_t s;
		load_sphere(scene_info.shape_data + shape_info->memory_start
			,&s);

		vector3f_t wc;
		point3f_t p_tmp,pcenter;vinit(pcenter,0,0,0);
		transform_point(p_tmp,s.o2w,pcenter);
		vsub(wc,*p,p_tmp);
		float dist_sqr = vdot(wc,wc);
		vsmul(wc,1.f/dist_sqr,wc);
		vector3f_t wc_x,wc_y;
		coordinate_system(&wc,&wc_x,&wc_y);
		if (dist_sqr - s.rad*s.rad < 1e-4f)
		{
			sphere_shape_sample(&s,scene_info,u0,u1,ns,pr);
			return;
		}
		float cos_theta_max = sqrt(max(0,1.f-s.rad*s.rad/dist_sqr));
		differential_geometry_t dg_sphere;
		float thit;
		point3f_t ps;
		ray_t r;
		vector3f_t r_dir;
		uniform_sample_cone(u0,u1,cos_theta_max,&wc_x,&wc_y,&wc,&r_dir);
		rinit(r,*p,r_dir);
		float epsilon;
		if(!intersect_sphere(scene_info.shape_data,shape_info->memory_start,&r,&thit,&dg_sphere,&epsilon))
		{
			vsmul(ps,s.rad,wc);
			vsub(ps,pcenter,ps);
		}
		else
			rpos(ps,r,thit);
		vsub(*ns,ps,pcenter);
		if(s.reverse_orientation)vsmul(*ns,-1.f,*ns);
		vassign(*pr,ps);
	}

}
#include "primitive.h"
INLINE float shape_pdf(shape_info_t shape_info,cl_scene_info_t scene_info,const point3f_t* p,const vector3f_t* wi)
{
	if (shape_info.shape_type == 0)
	{
	sphere_t s;
	load_sphere(scene_info.shape_data + shape_info.memory_start
		,&s);
	vector3f_t v_tmp ;
	point3f_t p_tmp,pcenter;vinit(pcenter,0,0,0);
	transform_point(p_tmp,s.o2w,pcenter);
	vsub(v_tmp,*p,p_tmp);
	float dist_sqr = vdot(v_tmp,v_tmp);
	//inside sphere
	if (dist_sqr - s.rad * s.rad < 0.00001f)
	{
		ray_t r;rinit(r,*p,*wi);
		intersection_t isect;
		float thit;
		differential_geometry_t dg;
		if(!(intersect_sphere(scene_info.shape_data,
			shape_info.memory_start,
			&r,&thit,&dg,&isect.ray_epsilon)))
		{
			return 0.f;
		}
		float wod = fabs(vdot(dg.nn,*wi));
		vsub(v_tmp,dg.p,*p);
		float dsqr = vdot(v_tmp,v_tmp);
		return (dsqr) / (4.f * FLOAT_PI * s.rad * s.rad)  * wod ;
	}
	else
	{
		float cosThetaMax = sqrt(max(0.f, (1.f - s.rad * s.rad /
			dist_sqr)));
		return 1.f / (2.f * FLOAT_PI * (1.f - cosThetaMax));
	}
	}

	return 0.f;
}
#endif
