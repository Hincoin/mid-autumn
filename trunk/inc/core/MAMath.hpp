#ifndef INCLUDE_MA_MATH_HPP
#define INCLUDE_MA_MATH_HPP

#include "MAConfig.hpp"
#include <utility>
#include <cmath>
#include <limits>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>

#include "TypeDefs.hpp"
#include "Vector.hpp"

#ifdef max
#undef max
#endif // max

#ifdef min
#undef min
#endif // min

namespace ma
{

	static const float INV_TWOPI = 1.f/(M_PI*2);
	static const float INV_PI = 0.318309886f;
    template<typename T1, typename T2>
    struct select_accurate_type_traits // select bigger type by default
    {
        typedef typename boost::mpl::if_c< (sizeof(T1)>sizeof(T2)),
        typename boost::mpl::if_c<std::numeric_limits<T1>::is_exact && !std::numeric_limits<T2>::is_exact,T2,T1>::type,
        typename boost::mpl::if_c<std::numeric_limits<T2>::is_exact && !std::numeric_limits<T1>::is_exact,T1,T2>::type >::type type;
    };

    // (Partial) specializations

    // Any combination with double will define a double
    template<typename T> struct select_accurate_type_traits<double, T>
    {
        typedef double type;
    };
    template<typename T> struct select_accurate_type_traits<T, double>
    {
        typedef double type;
    };
    // Avoid ambiguity for the double/double case
    template<> struct select_accurate_type_traits<double, double>
    {
        typedef double type;
    };

    // List other cases
    template<> struct select_accurate_type_traits<int, float>
    {
        typedef float type;
    };
    template<> struct select_accurate_type_traits<float, int>
    {
        typedef float type;
    };

    template<> struct select_accurate_type_traits<unsigned int, float>
    {
        typedef float type;
    };
    template<> struct select_accurate_type_traits<float, unsigned int>
    {
        typedef float type;
    };

    //extended ...


    namespace details
    {
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
        return details::equal_impl(x,y,(boost::mpl::bool_< boost::is_class<T1>::value || boost::is_class<T2>::value || (std::numeric_limits<T1>::is_exact && std::numeric_limits<T2>::is_exact) >*) (0));
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
    inline T smoothStep(T a, T b, T x)
    {
        if (x <= a)
            return 0;
        if (x >= b)
            return 1;
        float t = clamp((x - a) / (b - a), 0.0f, 1.0f);
        return t * t * (3 - 2 * t);
    }
    template<typename T>
    inline T linearStep(T a,T b, T x)
    {
        if (x <= a)
            return 0;
        if (x >= b)
            return 1;
        return (x - a)/(b-a);
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
        uint32 tmp = (uint32(IEEE_1_0 << 1) + IEEE_1_0 - *(uint32*)(&x)) >> 1;
        float y = *(float*)(&tmp);
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

template<typename T>
inline T reciprocal(const T v)
{
    return T(1) / v;
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
/*
    inline float linearStep(float a,float b, float x)
    {
        if (x <= a)
            return 0;
        if (x >= b)
            return 1;
#ifdef MA_FAST_MATH
        return (x - a)*reciprocal_approxim(b-a);
#else
        return (x-a)/(b-a);
#endif
    }*/
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
#elif defined(_MSC_VER) /*&& defined(__x86__)*/
        __asm fnclex;
#elif defined(__GNUC__) /*&& defined(__x86__)*/
        __asm__ __volatile__ ("fclex \n\t");
#else
#warn clearFPUException not supported.
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
	namespace details{
		static const float shift23 = static_cast<float>(1 << 23);
		static const float shift23_inv = 1.f/ static_cast<float>(1 << 23);
		struct fast_pow_impl
		{
			static float log2(float f)
			{
				static const float log_bodge = 0.346607f;
				float x =(float) *(int*)(&f);
				x *= shift23_inv;
				x-=127;
				float y = x-std::floor(x);
				y = (y-y*y)*log_bodge;
				return x+y;
			}
			static float pow2(float i)
			{
				static const float pow_bodge = 0.33971f;
				float y = i - std::floor(i);
				y = (y - y*y)*pow_bodge;
				float x = i + 127 - y;
				x*= shift23;
				*(int*)&x =(int)x;
				return x;
			}
		};
	}
    inline float fastPow(float x,float y)
    {
		return details::fast_pow_impl::pow2(y * details::fast_pow_impl::log2(x));
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
    inline int solveQuadric(double a, double b, double c,std::pair<double,double>& result)
    {
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
    inline double solveCubicForQuartic(double p, double q, double r)
    {
        double A2 = p * p;
        double Q = (A2 - 3.0 * q) / 9.0;
        double R = (p * (A2 - 4.5 * q) + 13.5 * r) / 27.0;
        double Q3 = Q * Q * Q;
        double R2 = R * R;
        double d = Q3 - R2;
        double an = p / 3.0;
        if (d >= 0)
        {
            d = R / std::sqrt(Q3);
            double theta = std::acos(d) / 3.0;
            double sQ = -2.0 * std::sqrt(Q);
            return sQ * std::cos(theta) - an;
        }
        else
        {
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
    inline int solveQuartic(double a, double b, double c, double d, double e,double results[4])
    {
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
        if (d1 < 0)
        {
            if (d1 > 1.0e-10)
                d1 = 0;
            else
                return 0;
        }
        double d2;
        if (d1 < 1.0e-10)
        {
            d2 = z * z - r;
            if (d2 < 0)
                return false;
            d2 = std::sqrt(d2);
        }
        else
        {
            d1 = std::sqrt(d1);
            d2 = 0.5 * q / d1;
        }
        // setup useful values for the quadratic factors
        double q1 = d1 * d1;
        double q2 = -0.25 * c1;
        double pm = q1 - 4 * (z - d2);
        double pp = q1 - 4 * (z + d2);
        if (pm >= 0 && pp >= 0)
        {
            // 4 roots (!)
            pm = std::sqrt(pm);
            pp = std::sqrt(pp);
            //double[] results = new double[4];
            results[0] = -0.5 * (d1 + pm) + q2;
            results[1] = -0.5 * (d1 - pm) + q2;
            results[2] = 0.5 * (d1 + pp) + q2;
            results[3] = 0.5 * (d1 - pp) + q2;
            // tiny insertion sort
            for (int i = 1; i < 4; i++)
            {
                for (int j = i; j > 0 && results[j - 1] > results[j]; j--)
                {
                    double t = results[j];
                    results[j] = results[j - 1];
                    results[j - 1] = t;
                }
            }
            //return results;
            return 4;
        }
        else if (pm >= 0)
        {
            pm = std::sqrt(pm);
            //double[] results = new double[2];
            results[0] = -0.5 * (d1 + pm) + q2;
            results[1] = -0.5 * (d1 - pm) + q2;
            //return results;
            return 2;
        }
        else if (pp >= 0)
        {
            pp = std::sqrt(pp);
            //double[] results = new double[2];
            results[0] = 0.5 * (d1 - pp) + q2;
            results[1] = 0.5 * (d1 + pp) + q2;
            return 2;
        }
        return 0;
    }

	inline float log2(float x) {
		static float invLog2 = 1.f / logf(2.f);
		return logf(x) * invLog2;
	}
	inline int log2Int(float v) {
		return ((*(int *) &v) >> 23) - 127;
	}
	inline bool isPowerOf2(int v) {
		return (v & (v - 1)) == 0;
	}
	inline unsigned RoundUpPow2(unsigned v) {
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		return v+1;
	}
}
#include <boost/random.hpp>


namespace ma{

	//[0,1)
	template<typename RealT>
	inline RealT genrand_real2(){
		typedef boost::random::lagged_fibonacci_01<RealT, 48, 607, 273> lagged_fibonacci607_t;
		typedef boost::uniform_real<RealT> uniform_real_t;
		static uniform_real_t uni_dist(0,1);
		static lagged_fibonacci607_t generator(5489UL);//default seed
		static boost::variate_generator<lagged_fibonacci607_t&, uniform_real_t > uni(generator, uni_dist);

		return uni();
	}
	//[m,M]
	inline unsigned long genrand_int32()
	{
		typedef boost::mt19937 generator_t;
		typedef boost::uniform_int<unsigned long> uniform_int_t;
		static uniform_int_t uni_dist(0,std::numeric_limits<unsigned long>::max());
		static generator_t generator(5489UL);
		static boost::variate_generator<generator_t&, uniform_int_t > uni(generator, uni_dist);

		return uni();
	}
	inline float RandomFloat()
	{
		return genrand_real2<float>();
	}
	inline unsigned long RandomUInt(){
		return genrand_int32();
	}
}

namespace ma{
	template<typename RealType>
	inline bool SolveLinearSystem2x2(const RealType A[2][2],
		const RealType B[2], RealType x[2]) {
			RealType det = A[0][0]*A[1][1] - A[0][1]*A[1][0];
			if (std::abs(det) < 1e-5)
				return false;
			RealType invDet = reciprocal(det);
			x[0] = (A[1][1]*B[0] - A[0][1]*B[1]) * invDet;
			x[1] = (A[0][0]*B[1] - A[1][0]*B[0]) * invDet;
			return true;
	}
	template<typename RealT>
	inline RealT Radians(RealT deg) {
		return ((RealT)M_PI/RealT(180)) * deg;
	}
	template<typename RealT>
	inline RealT Degrees(RealT rad) {
		return (RealT(180)/(RealT)M_PI) * rad;
	}


	inline void LatinHypercube(float *samples,
		int nSamples, int nDim) {
			// Generate LHS samples along diagonal
			float delta = 1.f / nSamples;
			for (int i = 0; i < nSamples; ++i)
				for (int j = 0; j < nDim; ++j)
					samples[nDim * i + j] = (i + RandomFloat()) * delta;
			// Permute LHS samples in each dimension
			for (int i = 0; i < nDim; ++i) {
				for (int j = 0; j < nSamples; ++j) {
					unsigned long other = RandomUInt() % nSamples;
					std::swap(samples[nDim * j + i],
						samples[nDim * other + i]);
				}
			}
	}

	//monte-carlo sample functions
	namespace mc{
	inline void ConcentricSampleDisk(float u1, float u2,
		float *dx, float *dy) {
			float r, theta;
			// Map uniform random numbers to $[-1,1]^2$
			float sx = 2 * u1 - 1;
			float sy = 2 * u2 - 1;
			// Map square to $(r,\theta)$
			// Handle degeneracy at the origin
			if (sx == 0.0 && sy == 0.0) {
				*dx = 0.0;
				*dy = 0.0;
				return;
			}
			if (sx >= -sy) {
				if (sx > sy) {
					// Handle first region of disk
					r = sx;
					if (sy > 0.0)
						theta = sy/r;
					else
						theta = 8.0f + sy/r;
				}
				else {
					// Handle second region of disk
					r = sy;
					theta = 2.0f - sx/r;
				}
			}
			else {
				if (sx <= sy) {
					// Handle third region of disk
					r = -sx;
					theta = 4.0f - sy/r;
				}
				else {
					// Handle fourth region of disk
					r = -sy;
					theta = 6.0f + sx/r;
				}
			}
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
			theta *= static_cast<float>( M_PI )/ 4.f;
			*dx = r*cosf(theta);
			*dy = r*sinf(theta);
	}


	template<typename RealT>
	inline typename vector_type<RealT,3>::type CosineSampleHemisphere(RealT u1, RealT u2) {
		typename vector_type<RealT,3>::type ret;
		ConcentricSampleDisk(u1, u2, &ret.x(), &ret.y());
		ret.z() = std::sqrt(std::max(0.f,
			1.f - ret.x()*ret.x() - ret.y()*ret.y()));
		return ret;
	}
	template<typename RealT>
	inline typename vector_type<RealT,3>::type UniformSampleHemisphere(RealT u1, RealT u2) {
		typedef typename vector_type<RealT,3>::type vector_t;
		float z = u1;
		float r = sqrtf(std::max(0.f, 1.f - z*z));
		float phi = 2 * M_PI * u2;
		float x = r * cosf(phi);
		float y = r * sinf(phi);
		return vector_t(x, y, z);
	}
	}
	using namespace mc;
}
#endif
