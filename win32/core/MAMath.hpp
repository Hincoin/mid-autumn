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

	template<typename T>
	inline T smoothStep(T a, T b, T x) {
		if (x <= a)
			return 0;
		if (x >= b)
			return 1;
		float t = clamp((x - a) / (b - a), 0.0f, 1.0f);
		return t * t * (3 - 2 * t);
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

	/**
	* Computes a fast approximation to <code>pow(a, b)</code>. Adapted
	* from <url>http://www.dctsystems.co.uk/Software/power.html</url>.
	* 
	* @param a a positive number
	* @param b a number
	* @return a^b
	*/
	inline float fastPow(float x,float y)
	{
		static const float shift23 = static_cast<float>(1 << 23);
		static const float shift23_inv = 1.f/ static_cast<float>(1 << 23);
		struct fast_pow_impl{
			static float log2(float f){
				static const float log_bodge = 0.346607f;
				float x =(float) *(int*)(&f);
				x *= shift23_inv;
				x-=127;
				float y = x-std::floor(x);
				y = (y-y*y)*log_bodge;
				return x+y;
			}
			static float pow2(float i){
				static const float pow_bodge = 0.33971f;
				float y = i - std::floor(i);
				y = (y - y*y)*pow_bodge;
				float x = i + 127 - y;
				x*= shift23;
				*(int*)&x =(int)x;
				return x;
			}
		};
		return fast_pow_impl::pow2(y * fast_pow_impl::log2(x));
	}

	//////////////////////////////////////////////////////////////////////////
	//equation solvers
	//the following code is taken from sunflow

	/**
	* Solves the equation ax^2+bx+c=0. Solutions are returned in a sorted array
	* if they exist.
	* 
	* @param a coefficient of x^2
	* @param b coefficient of x^1
	* @param c coefficient of x^0
	* @return an array containing the two real roots, or <code>null</code> if
	*         no real solutions exist
	*/
	inline int solveQuadric(double a, double b, double c,std::pair<double,double>& result) {
		double disc = b * b - 4 * a * c;
		if (disc < 0)
			return 0;
		disc = std::sqrt(disc);
		double q = ((b < 0) ? -0.5 * (b - disc) : -0.5 * (b + disc));
		double t0 = q / a;
		double t1 = c / q;
		// return sorted array
		result = (t0 > t1) ? std::make_pair(t1,t0) : std::make_pair(t0,t1);
		return 2;
	}

	/**
	* Return only one root for the specified cubic equation. This routine is
	* only meant to be called by the quartic solver. It assumes the cubic is of
	* the form: x^3+px^2+qx+r.
	* 
	* @param p
	* @param q
	* @param r
	* @return
	*/
	inline double solveCubicForQuartic(double p, double q, double r) {
		double A2 = p * p;
		double Q = (A2 - 3.0 * q) / 9.0;
		double R = (p * (A2 - 4.5 * q) + 13.5 * r) / 27.0;
		double Q3 = Q * Q * Q;
		double R2 = R * R;
		double d = Q3 - R2;
		double an = p / 3.0;
		if (d >= 0) {
			d = R / std::sqrt(Q3);
			double theta = std::acos(d) / 3.0;
			double sQ = -2.0 * std::sqrt(Q);
			return sQ * std::cos(theta) - an;
		} else {
			double sQ = std::pow(std::sqrt(R2 - Q3) + std::abs(R), 1.0 / 3.0);
			if (R < 0)
				return (sQ + Q / sQ) - an;
			else
				return -(sQ + Q / sQ) - an;
		}
	}


	/**
	* Solve a quartic equation of the form ax^4+bx^3+cx^2+cx^1+d=0. The roots
	* are returned in a sorted array of doubles in increasing order.
	* 
	* @param a coefficient of x^4
	* @param b coefficient of x^3
	* @param c coefficient of x^2
	* @param d coefficient of x^1
	* @param e coefficient of x^0
	* @return a sorted array of roots, or <code>null</code> if no solutions
	*         exist
	*/
	inline int solveQuartic(double a, double b, double c, double d, double e,double results[4]) {
		double inva = 1 / a;
		double c1 = b * inva;
		double c2 = c * inva;
		double c3 = d * inva;
		double c4 = e * inva;
		// cubic resolvant
		double c12 = c1 * c1;
		double p = -0.375 * c12 + c2;
		double q = 0.125 * c12 * c1 - 0.5 * c1 * c2 + c3;
		double r = -0.01171875 * c12 * c12 + 0.0625 * c12 * c2 - 0.25 * c1 * c3 + c4;
		double z = solveCubicForQuartic(-0.5 * p, -r, 0.5 * r * p - 0.125 * q * q);
		double d1 = 2.0 * z - p;
		if (d1 < 0) {
			if (d1 > 1.0e-10)
				d1 = 0;
			else
				return 0;
		}
		double d2;
		if (d1 < 1.0e-10) {
			d2 = z * z - r;
			if (d2 < 0)
				return false;
			d2 = std::sqrt(d2);
		} else {
			d1 = std::sqrt(d1);
			d2 = 0.5 * q / d1;
		}
		// setup useful values for the quadratic factors
		double q1 = d1 * d1;
		double q2 = -0.25 * c1;
		double pm = q1 - 4 * (z - d2);
		double pp = q1 - 4 * (z + d2);
		if (pm >= 0 && pp >= 0) {
			// 4 roots (!)
			pm = std::sqrt(pm);
			pp = std::sqrt(pp);
			//double[] results = new double[4];
			results[0] = -0.5 * (d1 + pm) + q2;
			results[1] = -0.5 * (d1 - pm) + q2;
			results[2] = 0.5 * (d1 + pp) + q2;
			results[3] = 0.5 * (d1 - pp) + q2;
			// tiny insertion sort
			for (int i = 1; i < 4; i++) {
				for (int j = i; j > 0 && results[j - 1] > results[j]; j--) {
					double t = results[j];
					results[j] = results[j - 1];
					results[j - 1] = t;
				}
			}
			//return results;
			return 4;
		} else if (pm >= 0) {
			pm = std::sqrt(pm);
			//double[] results = new double[2];
			results[0] = -0.5 * (d1 + pm) + q2;
			results[1] = -0.5 * (d1 - pm) + q2;
			//return results;
			return 2;
		} else if (pp >= 0) {
			pp = std::sqrt(pp);
			//double[] results = new double[2];
			results[0] = 0.5 * (d1 - pp) + q2;
			results[1] = 0.5 * (d1 + pp) + q2;
			return 2;
		}
		return 0;
	}


}

#endif