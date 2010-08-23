
#ifndef _MA_INCLUDED_MONTECARLO_HPP_
#define _MA_INCLUDED_MONTECARLO_HPP_
#include "MAMath.hpp"
namespace ma{
	template<typename RealType>
		RealType BalanceHeuristic(int nf,RealType fPdf,int ng,
				RealType gPdf)
		{
			return (nf * fPdf) / (nf * fPdf + ng * gPdf);
		}
	template<typename RealType>
		RealType PowerHeuristic(int nf,RealType fPdf,int ng,RealType gPdf)
		{
			RealType f = nf * fPdf,g = ng * gPdf;
			return (f*f) / (f*f + g*g);
		}
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
	template<typename VT,typename RealType>
		VT UniformSampleHemisphere(RealType u1,RealType u2)
		{
			RealType z = u1;
			RealType r = std::sqrt(std::max(0.f,1.f-z*z));
			RealType phi = 2 * M_PI * u2;
			RealType x = r * std::cos(phi);
			RealType y = r * std::sin(phi);
			return VT(x,y,z);
		}
	template<typename RealType>
		RealType UniformHemispherePdf(RealType theta,RealType phi)
		{
			return INV_TWOPI;
		}

	template<typename RealType>
	void ComputeStep1dCDF(RealType* f,int nSteps,RealType *c,
		RealType* cdf)
	{
		int i=0;
		cdf[i] = 0.;
		for (i = 1;i < nSteps+1;++i)
		{
			cdf[i] = cdf[i-1] + f[i-1] * reciprocal((RealType)nSteps);
		}
		*c = cdf[nSteps];
		for(i = 1;i < nSteps + 1; ++i)
			cdf[i] /= *c;
	}	
	template<typename RealType>
		RealType SampleStep1d(RealType* f,RealType *cdf,
				RealType c,
				RealType nSteps,RealType u,RealType &pdf)
		{
			RealType *ptr = std::lower_bound(cdf,cdf+(int)nSteps+1,u);
			int offset = (int)(ptr-cdf-1);
			u = (u - cdf[offset])/(cdf[offset + 1] - cdf[offset]);
			pdf = f[offset] / c;
			return (offset+u)/nSteps;
		}
	
}
#endif
