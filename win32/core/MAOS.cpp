#include "MAOS.hpp"

namespace Random{
	// our Randomizer is not really os specific, so we
	// code one for all, which should work on every platform the same,
	// which is desireable.

	static int seed = 0x0f0f0f0f;

	//! generates a pseudo random number
	int rand()
	{
		const int m = 2147483399;	// a non-Mersenne prime
		const int a = 40692;		// another spectral success story
		const int q = m/a;
		const int r = m%a;		// again less than q

		seed = a * (seed%q) - r* (seed/q);
		if (seed<0) seed += m;

		return seed;
	}

	//! resets the randomizer
	void reset(int s)
	{
		seed = s;
	}


}
