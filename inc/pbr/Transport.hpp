#ifndef _MA_INCLUDED_TRANSPORT_HPP_
#define _MA_INCLUDED_TRANSPORT_HPP_


namespace ma{
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
#endif