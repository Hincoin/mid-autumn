#ifndef _RANDOM_NUMBER_GENERATOR_MT19937_H_
#define _RANDOM_NUMBER_GENERATOR_MT19937_H_

// Random Number Declarations
class RandomNumberGeneratorMT19937 {
public:
    RandomNumberGeneratorMT19937(unsigned seed = 5489UL) {
        mti = N+1; /* mti==N+1 means mt[N] is not initialized */
        Seed(seed);
    }

    void Seed(unsigned seed) const;
    float RandomFloat() const;
    unsigned long RandomUnsignedInt() const;

private:
    static const int N = 624;
    mutable unsigned long mt[N]; /* the array for the state vector  */
    mutable int mti;
};


#endif