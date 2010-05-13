#ifndef __INCLUDE_MA_OS_HPP__
#define __INCLUDE_MA_OS_HPP__

namespace Random
{
	//! resets the random number generator
	void reset(int s = 0x0f0f0f0f);
	//! generates a pseudo random number
	int rand();
};
#endif

