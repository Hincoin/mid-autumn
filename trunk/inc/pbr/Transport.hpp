#ifndef _MA_INCLUDED_TRANSPORT_HPP_
#define _MA_INCLUDED_TRANSPORT_HPP_


#include "CRTPInterfaceMacro.hpp"
#include "ptr_var.hpp"
namespace ma{
	namespace integrator{
		DECL_FUNC_NEST(spectrum_t,li,4)
		DECL_FUNC(void,preprocess,1)
		DECL_FUNC(void,requestSamples,2)	
	}
	BEGIN_CRTP_INTERFACE(Integrator)
		ADD_CRTP_INTERFACE_TYPEDEF(spectrum_t);
		ADD_CRTP_INTERFACE_TYPEDEF(scene_ptr);
		ADD_CRTP_INTERFACE_TYPEDEF(ray_differential_t);
		ADD_CRTP_INTERFACE_TYPEDEF(sample_ptr);
		ADD_CRTP_INTERFACE_TYPEDEF(scalar_t);
		ADD_CRTP_INTERFACE_TYPEDEF(sample_t);

		CRTP_CONST_METHOD(spectrum_t,li,4,
			( I_(const scene_ptr,scene), I_(const ray_differential_t&,r), I_(const sample_ptr,sample), I_(scalar_t&,alpha)));

		CRTP_METHOD(void,preprocess,1,( I_(const scene_ptr,s)));
		CRTP_METHOD(void,requestSamples,2,( I_(sample_t&,s), I_(const scene_ptr,scene)));
		END_CRTP_INTERFACE
}

namespace ma{
	namespace details{
	template<typename I>
	struct integrator_creator;	
	}
	template <typename I>
		I* create_integrator(const ParamSet& param)
		{
			return details::integrator_creator<I>()(param);
		}
}
#endif
