#ifndef _RANDOM_NUMBER_GENERATOR_H_
#define _RANDOM_NUMBER_GENERATOR_H_
#include "config.h"

INLINE float GetRandom(unsigned int *seed0, unsigned int *seed1) {
	*seed0 = 36969 * ((*seed0) & 65535) + ((*seed0) >> 16);
	*seed1 = 18000 * ((*seed1) & 65535) + ((*seed1) >> 16);

	unsigned int ires = ((*seed0) << 16) + (*seed1);

	/* Convert to float */
	union {
		float f;
		unsigned int ui;
	} res;
	res.ui = (ires & 0x007fffff) | 0x40000000;

	return (res.f - 2.f) / 2.f;
}


struct Seed{
	unsigned s1, s2, s3;
	} ;

 //------------------------------------------------------------------------------
// Random number generator
 // maximally equidistributed combined Tausworthe generator
//------------------------------------------------------------------------------

 #define FLOATMASK 0x00ffffffu

 INLINE unsigned TAUSWORTHE(const unsigned s, const unsigned a,
					 	const unsigned b, const unsigned c,
					 	const unsigned d) {
						 	return ((s&c)<<d) ^ (((s << a) ^ s) >> b);
						 }
 
INLINE unsigned LCG(const unsigned x) { return x * 69069; }
 
INLINE unsigned ValidSeed(const unsigned x, const unsigned m) {
	 	return (x < m) ? (x + m) : x;
	 }
 
INLINE void init_rng(unsigned seed, Seed *s) {
	 	// Avoid 0 value
		 	seed = (seed == 0) ? (seed + 0xffffffu) : seed;
		 	s->s1 = ValidSeed(LCG(seed), 1);
	 	s->s2 = ValidSeed(LCG(s->s1), 7);
	 	s->s3 = ValidSeed(LCG(s->s2), 15);
	 }
 
INLINE unsigned long random_uint(Seed *s) {
	 	s->s1 = TAUSWORTHE(s->s1, 13, 19, 4294967294UL, 12);
	 	s->s2 = TAUSWORTHE(s->s2, 2, 25, 4294967288UL, 4);
	 	s->s3 = TAUSWORTHE(s->s3, 3, 11, 4294967280UL, 17);
	 
		 	return ((s->s1) ^ (s->s2) ^ (s->s3));
	 }
 
INLINE float random_float(Seed *s) {
	 	return (random_uint(s) & FLOATMASK) * (1.f / (FLOATMASK + 1UL));
	 }

#endif