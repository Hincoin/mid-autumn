#ifndef INCLUDE_MA_MATH_HPP
#define INCLUDE_MA_MATH_HPP

#include "MAConfig.hpp"
#include <utility>
#include <cmath>
namespace ma{

	//! returns linear interpolation of a and b with ratio t
	//! \return: a if t==0, b if t==1, and the linear interpolation else
	template<class T>
	inline T lerp(const T& a, const T& b, const float t)
	{
		return (T)(a*(1.f-t)) + (b*t);
	}

	//! clamps a value between low and high
	template <class T>
	inline const T clamp (const T& value, const T& low, const T& high)
	{
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif
		return std::min (std::max(value,low), high);
	}


	//! code is taken from IceFPU
	//! Integer representation of a floating-point value.
#define IR(x)				((uint32&)(x))

	//! Absolute integer representation of a floating-point value
#define AIR(x)				(IR(x)&0x7fffffff)

	//! Floating-point representation of an integer value.
#define FR(x)				((float&)(x))


	//! integer representation of 1.0
#define IEEE_1_0			0x3f800000
	//! integer representation of 255.0
#define IEEE_255_0			0x437f0000

	inline float reciprocal_squareroot(const float x)
	{
#ifdef MA_FAST_MATH
		// comes from Nvidia
#if 1
		uint32 tmp = (uint32(IEEE_1_0 << 1) + IEEE_1_0 - *(uint32*)&x) >> 1;
		float y = *(float*)&tmp;
		return y * (1.47f - 0.47f * x * y * y);
#elif defined(_MSC_VER)
		// an sse2 version
		__asm
		{
			movss	xmm0, x
				rsqrtss	xmm0, xmm0
				movss	x, xmm0
		}
		return x;
#endif
#else // no fast math
		return 1.f / sqrtf ( x );
#endif
	}



	inline float reciprocal ( const float f )
	{
#ifdef MA_FAST_MATH
		//! i do not divide through 0.. (fpu expection)
		// instead set f to a high value to get a return value near zero..
		// -1000000000000.f.. is use minus to stay negative..
		// must test's here (plane.normal dot anything ) checks on <= 0.f
		return 1.f / f;
		//uint32 x = (-(AIR(f) != 0 ) >> 31 ) & ( IR(f) ^ 0xd368d4a5 ) ^ 0xd368d4a5;
		//return 1.f / FR ( x );
#else // no fast math
		return 1.f / f;
#endif
	}


	inline float reciprocal_approxim ( const float p )
	{
#ifdef MA_FAST_MATH
		register uint32 x = 0x7F000000 - IR ( p );
		const float r = FR ( x );
		return r * (2.0f - p * r);
#else // no fast math
		return 1.f / p;
#endif
	}


	inline int32 floor32(float x)
	{
#ifdef MA_FAST_MATH
		const float h = 0.5f;

		int32 t;

#if defined(_MSC_VER)
		__asm
		{
			fld	x
				fsub	h
				fistp	t
		}
#elif defined(__GNUC__)
		__asm__ __volatile__ (
			"fsub %2 \n\t"
			"fistpl %0"
			: "=m" (t)
			: "t" (x), "f" (h)
			: "st"
			);
#else
#  warn MA_FAST_MATH not supported.
		return (int32) floorf ( x );
#endif
		return t;
#else // no fast math
		return (int32) floorf ( x );
#endif
	}


	inline int32 ceil32 ( float x )
	{
#ifdef MA_FAST_MATH
		const float h = 0.5f;

		int32 t;

#if defined(_MSC_VER)
		__asm
		{
			fld	x
				fadd	h
				fistp	t
		}
#elif defined(__GNUC__)
		__asm__ __volatile__ (
			"fadd %2 \n\t"
			"fistpl %0 \n\t"
			: "=m"(t)
			: "t"(x), "f"(h)
			: "st"
			);
#else
#  warn MA_FAST_MATH not supported.
		return (int32) ceilf ( x );
#endif
		return t;
#else // not fast math
		return (int32) ceilf ( x );
#endif
	}

	inline float round( float x )
	{
		return floorf( x + 0.5f );
	}


	inline int32 round32(float x)
	{
#if defined(MA_FAST_MATH)
		int32 t;

#if defined(_MSC_VER)
		__asm
		{
			fld   x
				fistp t
		}
#elif defined(__GNUC__)
		__asm__ __volatile__ (
			"fistpl %0 \n\t"
			: "=m"(t)
			: "t"(x)
			: "st"
			);
#else
#  warn MA_FAST_MATH not supported.
		return (int32) round(x);
#endif
		return t;
#else // no fast math
		return (int32) round(x);
#endif
	}

	inline void clearFPUException ()
	{
#ifdef MA_FAST_MATH
#ifdef feclearexcept
		feclearexcept(FE_ALL_EXCEPT);
#elif defined(_MSC_VER)
		__asm fnclex;
#elif defined(__GNUC__) && defined(__x86__)
		__asm__ __volatile__ ("fclex \n\t");
#else
#  warn clearFPUException not supported.
#endif
#endif
	}

}

#endif