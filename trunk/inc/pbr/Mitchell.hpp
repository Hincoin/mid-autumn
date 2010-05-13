#ifndef _MA_INCLUDED_MITCHELL_HPP_
#define _MA_INCLUDED_MITCHELL_HPP_
#include "Sampling.hpp"
namespace ma{
	// Mitchell Filter Declarations
	template<typename Conf>
	class MitchellFilter : public Filter<MitchellFilter<Conf>,typename Conf::interface_config> {
		typedef Filter<MitchellFilter<Conf>,typename Conf::interface_config> parent_type;
		ADD_SAME_TYPEDEF(Conf,scalar_t)
	public:
		// MitchellFilter Public Methods
		MitchellFilter(scalar_t b, scalar_t c, scalar_t xw, scalar_t yw)
			: parent_type(xw, yw) { B = b; C = c; }
		scalar_t evaluateImpl(scalar_t x, scalar_t y) const;
	private:
		scalar_t Mitchell1D(scalar_t x) const {
			x = fabsf(2.f * x);
			if (x > 1.f)
				return ((-B - 6*C) * x*x*x + (6*B + 30*C) * x*x +
				(-12*B - 48*C) * x + (8*B + 24*C)) * (1.f/6.f);
			else
				return ((12 - 9*B - 6*C) * x*x*x +
				(-18 + 12*B + 6*C) * x*x +
				(6 - 2*B)) * (1.f/6.f);
		}
	private:
		scalar_t B, C;
	};
	// Mitchell Filter Method Definitions
		template<typename Conf>
		typename Conf::scalar_t MitchellFilter<Conf>::evaluateImpl(scalar_t x, scalar_t y) const {
		return Mitchell1D(x * parent_type::invXWidth) *
			Mitchell1D(y * parent_type::invYWidth);
	}
}

namespace ma
{
	MAKE_TYPE_STR_MAP(1,MitchellFilter,mitchell)
namespace details
{
template<typename Conf>
	struct filter_creator<MitchellFilter<Conf> >
	{
		typedef MitchellFilter<Conf> filter_t;
		filter_t*
			operator()(const ParamSet& param)const
			{
				typedef typename Conf::scalar_t scalar_t;
				scalar_t xw = param.as<scalar_t>("xwidth",2.f);
				scalar_t yw = param.as<scalar_t>("ywidth",2.f);
				scalar_t B = param.as<scalar_t> ("B",1.f/3.f);
				scalar_t C = param.as<scalar_t> ("C",1.f/3.f);
				return new filter_t(B,C,xw,yw);
			}
	};
}
}
#endif
