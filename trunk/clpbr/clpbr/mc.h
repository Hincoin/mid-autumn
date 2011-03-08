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
									   float r, theta;
									   // Map uniform random numbers to $[-1,1]^2$
									   float sx = 2 * u1 - 1;
									   float sy = 2 * u2 - 1;
									   // Map square to $(r,\theta)$
									   // Handle degeneracy at the origin
									   if (sx == 0.0 && sy == 0.0) {
										   *dx = 0.0;
										   *dy = 0.0;
										   return;
									   }
									   if (sx >= -sy) {
										   if (sx > sy) {
											   // Handle first region of disk
											   r = sx;
											   if (sy > 0.0)
												   theta = sy/r;
											   else
												   theta = 8.0f + sy/r;
										   }
										   else {
											   // Handle second region of disk
											   r = sy;
											   theta = 2.0f - sx/r;
										   }
									   }
									   else {
										   if (sx <= sy) {
											   // Handle third region of disk
											   r = -sx;
											   theta = 4.0f - sy/r;
										   }
										   else {
											   // Handle fourth region of disk
											   r = -sy;
											   theta = 6.0f + sx/r;
										   }
									   }
									   theta *= FLOAT_PI / 4.f;
									   *dx = r*cos(theta);
									   *dy = r*sin(theta);
}



INLINE void cosine_sample_hemisphere(float u1, float u2,vector3f_t *ret) {
	concentric_sample_disk(u1, u2, &ret->x, &ret->y);
	ret->z = sqrt(max(0.f,
	                  (1.f - ret->x*ret->x - ret->y*ret->y)));
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
	cdf[0] = 0.;
	for (i = 1; i < nSteps+1; ++i)
		cdf[i] = cdf[i-1] + f[i-1] / nSteps;
	// Transform step function integral into cdf
	*c = cdf[nSteps];
	for (i = 1; i < nSteps+1; ++i)
		cdf[i] /= *c;
}
INLINE float sample_step_1d(float *f, float *cdf, float c,
		int nSteps, float u, float *pdf) {
	// Find surrounding cdf segments
	float *ptr = cdf;
	c_lower_bound(ptr, cdf+nSteps+1, u);
	int offset = (int) (ptr-cdf-1);
	// Return offset along current cdf segment
	u = (u - cdf[offset]) / (cdf[offset+1] - cdf[offset]);
	*pdf = f[offset] / c;
	return (offset + u) / nSteps;
}
#endif