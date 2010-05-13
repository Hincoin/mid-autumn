
#ifndef _MA_INCLUDED_MONTECARLO_HPP_
#define _MA_INCLUDED_MONTECARLO_HPP_
#include "MAMath.hpp"
namespace ma{
	template<typename scalar_t>
	inline typename vector_type<scalar_t,3>::type UniformSampleSphere(scalar_t u1,scalar_t u2)
	{
		typedef typename vector_type<scalar_t,3>::type ret_type;
		scalar_t z = scalar_t(1) - scalar_t( 2 )* u1;
		scalar_t r = std::sqrt(std::max(scalar_t(0),scalar_t(1) - z * z));
		scalar_t phi = scalar_t(2) * M_PI * u2;
		scalar_t x = r * std::cos(phi);
		scalar_t y = r * std::sin(phi);
		return ret_type(x,y,z);
	}
	template<typename RealType>
	inline RealType UniformSpherePdf()
	{
		return RealType(1) * reciprocal(RealType(4) * M_PI);
	}
}
#endif
