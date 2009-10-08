#ifndef _MA_INCLUDED_LIGHT_HPP_
#define _MA_INCLUDED_LIGHT_HPP_

#include "CRTPInterfaceMacro.hpp"
//#include "Move.hpp"
namespace ma
{
	BEGIN_CRTP_INTERFACE(Light)
		//type dependencies
		ADD_CRTP_INTERFACE_TYPEDEF(transform_t)
		ADD_CRTP_INTERFACE_TYPEDEF(spectrum_t)
		ADD_CRTP_INTERFACE_TYPEDEF(ray_differential_t)
		ADD_CRTP_INTERFACE_TYPEDEF(scene_ptr)
		ADD_CRTP_INTERFACE_TYPEDEF(vector_t)
		ADD_CRTP_INTERFACE_TYPEDEF(normal_t)
		ADD_CRTP_INTERFACE_TYPEDEF(point_t)
		ADD_CRTP_INTERFACE_TYPEDEF(scalar_t);
		ADD_CRTP_INTERFACE_TYPEDEF(visibility_tester_t);
		ADD_CRTP_INTERFACE_TYPEDEF(ray_t)

public:
	Light(const transform_t& l2w,int ns=1):num_samples(std::max(1,ns)),
		light_to_world(l2w),world_to_light(ma::force_move(l2w.inverse())){}

	CRTP_CONST_METHOD(spectrum_t,sample_l,3,( I_(const point_t&,p), I_(vector_t&,wi), I_(visibility_tester_t&,vis)));
	CRTP_CONST_METHOD(spectrum_t,power,1,( I_(const scene_ptr,s)));
	CRTP_CONST_METHOD(bool,isDeltaLight,0,());
	//CRTP_CONST_METHOD(spectrum_t,le,1,( (const ray_differential_t&,r)));
	spectrum_t le(const ray_differential_t& r)const{return spectrum_t();}
	CRTP_CONST_METHOD(spectrum_t,sample_l,6,
		( I_(const point_t&,p),
		 I_(scalar_t,u1),
		 I_(scalar_t,u2),
		 I_(vector_t&,wi),
		 I_(scalar_t&,pdf),
		 I_(visibility_tester_t&,vis)
		));
	CRTP_CONST_METHOD(scalar_t,pdf,2,( I_(const point_t&,p), I_(const vector_t&,wi)));
	CRTP_CONST_METHOD(spectrum_t,sample_l,7,(
		 I_(const scene_ptr,s),
		 I_(scalar_t,u1),
		 I_(scalar_t,u2),
		 I_(scalar_t,u3),
		 I_(scalar_t,u4),
		 I_(ray_t&,r),
		 I_(scalar_t&,pdf)
		));

	spectrum_t sample_l(const point_t& p,const normal_t& ,
		scalar_t u1,scalar_t u2,vector_t& wi,scalar_t &pdf,visibility_tester_t& vis)const{
			return sample_l(p,u1,u2,wi,pdf,vis);
	}

	scalar_t pdf(const point_t& p,const normal_t& ,const vector_t& wi)const
	{
		return pdf(p,wi);
	}
protected:
	const int num_samples;
	const transform_t light_to_world,world_to_light;
	END_CRTP_INTERFACE

template<typename Conf>
	struct VisibilityTester{
		ADD_SAME_TYPEDEF(Conf,point_t);
		ADD_SAME_TYPEDEF(Conf,scene_ptr)
		ADD_SAME_TYPEDEF(Conf,vector_t);
		ADD_SAME_TYPEDEF(Conf,ray_t)
		ADD_SAME_TYPEDEF(Conf,spectrum_t)
		// VisibilityTester Public Methods
		void setSegment(const point_t &p1, const point_t &p2) {
			r = ray_t(p1, p2-p1, ray_t::epsilon, 1.f -  ray_t::epsilon);
		}
		void setRay(const point_t &p, const vector_t &w) {
			r = ray_t(p, w, ray_t::epsilon);
		}
		bool unOccluded(const scene_ptr scene) const
		{
			return !scene->intersect_p(r);
		}
		spectrum_t transmittance(const scene_ptr scene) const{
			return scene->transmittance(r);
		}
		ray_t r;
	};
}
#include "ParamSet.hpp"

namespace ma{
	namespace details{
		template<typename L>
		struct light_creator;
	}
	template<typename L>
	L* create_light(const typename L::transform_t &light2world,
		const ParamSet &paramSet){
			return details::light_creator<L>()(light2world,paramSet);
	}
}


#endif
