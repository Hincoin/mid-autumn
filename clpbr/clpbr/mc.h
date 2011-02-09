#ifndef _MC_H_
#define _MC_H_

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

#endif