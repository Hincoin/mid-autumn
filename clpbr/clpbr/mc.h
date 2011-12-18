#ifndef _MC_H_
#define _MC_H_

#include "calgorithm.h"

// MC Inline Functions
INLINE float balance_heuristic(int nf, float fPdf, int ng,
							  float gPdf) {
								  return (nf * fPdf) / (nf * fPdf + ng * gPdf);
}
INLINE float power_heuristic(int nf, float fPdf, int ng,
							float gPdf) {
								float f = nf * fPdf, g = ng * gPdf;
								return (f*f) / (f*f + g*g);
}
INLINE void concentric_sample_disk(float u1, float u2,
								   float *dx, float *dy) {
#ifndef FLOAT_PI
#define FLOAT_PI 3.1415926f
#endif
									   //LZY: From Pete Shirley's Graphic blog
									   float r, theta;
									   // Map uniform random numbers to $[-1,1]^2$
									   float sx = 2 * u1 - 1;
									   float sy = 2 * u2 - 1;
									   if( sx * sx > sy * sy)
									   {
										   r = sx;
										   theta  = (FLOAT_PI/4.f) * (sy/sx);
									   }
									   else
									   {
										   r = sy;
										   theta = -(FLOAT_PI/4.f) * (sx/sy) + (FLOAT_PI / 2.f);
									   }
									   *dx = r*cos(theta);
									   *dy = r*sin(theta);
}



INLINE void cosine_sample_hemisphere(float u1, float u2,vector3f_t *ret) {
	float x,y;
	concentric_sample_disk(u1, u2, &x, &y);
	(*ret).x = x;
	(*ret).y = y;
	(*ret).z = sqrt(max(0.f,
	                  (1.f - (*ret).x*(*ret).x - (*ret).y*(*ret).y)));
}
INLINE void uniform_sample_cone(float u1, float u2, float cos_theta_max,
						 const vector3f_t *x, const vector3f_t *y, const vector3f_t *z,vector3f_t* ret) {
							 float costheta = cos_theta_max + u1*(1.f-cos_theta_max);//lerp(u1, costhetamax, 1.f);
							 float sintheta = sqrt(1.f - costheta*costheta);
							 float phi = u2 * 2.f * FLOAT_PI;
							 vector3f_t v_tmp;
							 vsmul(v_tmp,cos(phi) * sintheta,*x);
							 vsmul(*ret,sin(phi) * sintheta,*y);
							 vadd(*ret,*ret,v_tmp);
							 vsmul(v_tmp,costheta,*z);
							 vadd(*ret,*ret,v_tmp);
}
INLINE float uniform_cone_pdf(float cos_theta_max)
{
	return 1.f / (2.f * FLOAT_PI * (1.f - cos_theta_max));
}
INLINE void uniform_sample_hemisphere(float u1,float u2,vector3f_t *ret)
{
	float z = u1;
	float r = sqrt(max(0.f, 1.f - z*z));
	float phi = 2 * FLOAT_PI * u2;
	float x = r * cos(phi);
	float y = r * sin(phi);
	vinit(*ret,x,y,z);
}
// MC Function Definitions
INLINE void compute_step_1d_cdf(float *f, int nSteps, float *c,
		float *cdf) {
	// Compute integral of step function at $x_i$
	int i;
	cdf[0] = 0.f;
	for (i = 1; i < nSteps+1; ++i)
		cdf[i] = cdf[i-1] + f[i-1] / nSteps;
	// Transform step function integral into cdf
	*c = cdf[nSteps];
	for (i = 1; i < nSteps+1; ++i)
		cdf[i] /= *c;
}
INLINE float sample_step_1d(CONSTANT float *f,CONSTANT float *cdf, float c,
		int nSteps, float u, float *pdf) {
	// Find surrounding cdf segments
	CONSTANT float *ptr = cdf;
	c_lower_bound(ptr, cdf+nSteps+1, u);
	int offset = max(0,(int) (ptr-cdf-1));
	// Return offset along current cdf segment
	u = (u - cdf[offset]) / (cdf[offset+1] - cdf[offset]);
	*pdf = f[offset] / c;
	return (offset + u) / nSteps;
}
INLINE void uniform_sample_triangle(float ud1,float ud2,float *u, float *v)
{
	float su1 = sqrt(ud1);
	*u = 1.f - su1;
	*v = ud2 * su1;
}
#endif