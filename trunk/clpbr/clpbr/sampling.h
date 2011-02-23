#ifndef _SAMPLING_H_
#define _SAMPLING_H_

#include "config.h"
#include "random_number_generator.h"

INLINE float radical_inverse(int n, int base)
{
	float val = 0.f;
	float inv_base = 1.f/base,inv_bi = inv_base;
	while (n>0)
	{
		//compute next digit of radical inverse
		int d_i = (n%base);
		val += d_i * inv_bi;
		n /= base;
		inv_bi *= inv_base;
	}
	return val;
}
INLINE void latin_hypercube(float *samples,int nSamples, int nDim,Seed* seed) {
	// Generate LHS samples along diagonal
	float delta = 1.f / nSamples;
	for (int i = 0; i < nSamples; ++i)
		for (int j = 0; j < nDim; ++j)
			samples[nDim * i + j] = (i + random_float(seed)) * delta;
	// Permute LHS samples in each dimension
	for (int i = 0; i < nDim; ++i) {
		for (int j = 0; j < nSamples; ++j) {
			unsigned other = random_uint(seed) % nSamples;
			float tmp = samples[nDim * j + i];
			samples[nDim * j + i] = samples[nDim * other + i];
			samples[nDim * other + i] = tmp;
		}
	}
}
#endif