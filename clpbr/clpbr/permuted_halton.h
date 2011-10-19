#ifndef _PERMUTATED_HALTON_H_
#define _PERMUTATED_HALTON_H_

#include "random_number_generator_mt19937.h"
#include <utility>
template <typename T>
void Shuffle(T *samp, unsigned count, unsigned dims, RandomNumberGeneratorMT19937 &rng) {
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
        val += d_i * invBi;
        n *= invBase;
        invBi *= invBase;
    }
    return val;
}

static const float OneMinusEpsilon=0.9999999403953552f;
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



#endif