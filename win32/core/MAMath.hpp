#ifndef INCLUDE_MA_MATH_HPP
#define INCLUDE_MA_MATH_HPP

#include "MAConfig.hpp"
#include <utility>
#include <cmath>
#include <limits>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
namespace ma{

	template<typename T1, typename T2>
	struct select_accurate_type_traits // select bigger type by default
	{
		typedef typename boost::mpl::if_c< (sizeof(T1)>sizeof(T2)), 
			typename boost::mpl::if_c<std::numeric_limits<T1>::is_exact && !std::numeric_limits<T2>::is_exact,T2,T1>::type,
			typename boost::mpl::if_c<std::numeric_limits<T2>::is_exact && !std::numeric_limits<T1>::is_exact,T1,T2>::type >::type type;
	};

	// (Partial) specializations

	// Any combination with double will define a double
	template<typename T> struct select_accurate_type_traits<double, T> { typedef double type; };
	template<typename T> struct select_accurate_type_traits<T, double> { typedef double type; };
	// Avoid ambiguity for the double/double case
	template<> struct select_accurate_type_traits<double, double> { typedef double type; };

	// List other cases
	template<> struct select_accurate_type_traits<int, float> { typedef float type; };
	template<> struct select_accurate_type_traits<float, int> {	typedef float type; };

	template<> struct select_accurate_type_traits<unsigned int, float> { typedef float type; };
	template<> struct select_accurate_type_traits<float, unsigned int> {	typedef float type; };

	//extended ...


	namespace details{

		template<typename T1,typename T2>
		inline bool equal_impl(const T1& x, const T2& y, boost::mpl::bool_<true>* )
		{
			return x == y;
		}
		template<typename T1,typename T2>
		inline bool equal_impl(const T1& x, const T2& y, boost::mpl::bool_<false>* )
		{
			typedef typename select_accurate_type_traits<T1,T2>::type T;
#ifdef __GNUC__
			return fabs(x - y) < std::numeric_limits<T>::epsilon();
#else
			return abs(x - y) < std::numeric_limits<T>::epsilon();
#endif
		}
	}
	template<typename T1,typename T2>
	inline bool equal(const T1& x, const T2& y)
	{
		return details::equal_impl(x,y,(boost::mpl::bool_< (std::numeric_limits<T1>::is_exact && std::numeric_limits<T2>::is_exact) >*) (0));
	}
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