#ifndef _PERMUTATED_HALTON_H_
#define _PERMUTATED_HALTON_H_


#define OneMinusEpsilon 0.9999999403953552f

typedef struct 
{
    unsigned b[16], permute[381];
}permuted_halton16_t;

INLINE float permuted_radical_inverse(unsigned n, unsigned base,
                                     GLOBAL unsigned *p) {
    float val = 0;
    float invBase = 1.f / base, invBi = invBase;

    while (n > 0) {
        unsigned d_i = p[n % base];
        val += (float)(d_i) * invBi;
        n = (unsigned int)(n * invBase);
        invBi *= invBase;
    }
    return val;
}
INLINE void halton16_sample(GLOBAL permuted_halton16_t *halton, unsigned n,float *out)
{
	GLOBAL unsigned *p = halton->permute;
	for (unsigned i = 0; i < 16; ++i) {
		out[i] = min((float)(permuted_radical_inverse(n, halton->b[i], p)), 
			OneMinusEpsilon);
		p += halton->b[i];
	}
}
#ifndef CL_KERNEL
#include "random_number_generator_mt19937.h"
#include <utility>
template <typename T>
inline void Shuffle(T *samp, unsigned count, unsigned dims, RandomNumberGeneratorMT19937 &rng) {
    for (unsigned i = 0; i < count; ++i) {
        unsigned other = i + (rng.RandomUnsignedInt() % (count - i));
        for (unsigned j = 0; j < dims; ++j)
			std::swap(samp[dims*i + j], samp[dims*other + j]);
    }
}

inline void GeneratePermutation(unsigned *buf, unsigned b, RandomNumberGeneratorMT19937 &rng) {
    for (unsigned i = 0; i < b; ++i)
        buf[i] = i;
    Shuffle(buf, b, 1, rng);
}


inline double PermutedRadicalInverse(unsigned n, unsigned base,
                                     const unsigned *p) {
    double val = 0;
    double invBase = 1. / base, invBi = invBase;

    while (n > 0) {
        unsigned d_i = p[n % base];
        val += double(d_i) * invBi;
        n = unsigned int(n * invBase);
        invBi *= invBase;
    }
    return val;
}

class PermutedHalton {
public:
    // PermutedHalton Public Methods
    PermutedHalton(unsigned d, RandomNumberGeneratorMT19937 &rng);
    ~PermutedHalton() {
        delete[] b;
        delete[] permute;
    }
    void Sample(unsigned n, float *out) const {
        unsigned *p = permute;
        for (unsigned i = 0; i < dims; ++i) {
            out[i] = min(float(PermutedRadicalInverse(n, b[i], p)), 
                         OneMinusEpsilon);
            p += b[i];
        }
    }
private:
    // PermutedHalton Private Data
    unsigned dims;
    unsigned *b, *permute;
    PermutedHalton(const PermutedHalton &);
    PermutedHalton &operator=(const PermutedHalton &);
};

void permuted_halton16_initialize(permuted_halton16_t *halton, RandomNumberGeneratorMT19937 &rng);

#endif

#endif