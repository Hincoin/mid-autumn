#ifndef _MATH_H_
#define _MATH_H_

#include "config.h"

INLINE int Quadratic(float A, float B, float C, float *t0,
			  float *t1) {
				  // Find quadratic discriminant
				  float discrim = B * B - 4.f * A * C;
				  if (discrim < 0.) return 0;
				  float rootDiscrim = sqrt(discrim);
				  // Compute quadratic _t_ values
				  float q;
				  if (B < 0) q = -.5f * (B - rootDiscrim);
				  else       q = -.5f * (B + rootDiscrim);
				  *t0 = q / A;
				  *t1 = C / q;
				  if (*t0 > *t1) {
					  float t = *t0;
					  *t0 = *t1;
					  *t1 = t;
				  }
				  return 1;
}



#endif