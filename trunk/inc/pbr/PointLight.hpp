
#ifndef _MA_INCLUDED_POINTLIGHT_HPP_
#define _MA_INCLUDED_POINTLIGHT_HPP_
#include "MonteCarlo.hpp"
#include "Light.hpp"

namespace ma{
	template<typename Conf>
	class PointLight: public Light<PointLight<Conf>,typename Conf::interface_config>
	{
		friend class Light<PointLight<Conf>,typename Conf::interface_config>;
		typedef Light<PointLight<Conf>,typename Conf::interface_config> parent_type;
	public:
		ADD_SAME_TYPEDEF(Conf,transform_t);
		ADD_SAME_TYPEDEF(Conf,spectrum_t);
		ADD_SAME_TYPEDEF(Conf,visibility_tester_t);
		ADD_SAME_TYPEDEF(Conf,scene_ptr);
		ADD_SAME_TYPEDEF(Conf,point_t);
		ADD_SAME_TYPEDEF(Conf,vector_t);
		ADD_SAME_TYPEDEF(Conf,scalar_t);
		ADD_SAME_TYPEDEF(Conf,ray_t);
	private:
		point_t light_pos;
		spectrum_t intensity;
	public:
		PointLight(const transform_t& light2world,const spectrum_t& intensity);
	private:
		bool isDeltaLightImpl()const{return true;}
		spectrum_t sample_lImpl(const point_t& p,scalar_t u1,scalar_t u2,
			vector_t& wo,scalar_t &pdf,visibility_tester_t& vt)const;
		spectrum_t sample_lImpl(const scene_ptr scene, scalar_t u1, scalar_t u2,
			scalar_t u3, scalar_t u4, ray_t& ray, scalar_t& pdf) const;
		spectrum_t sample_lImpl(const point_t& p,vector_t& wi,visibility_tester_t& vis)const;
		spectrum_t powerImpl(const scene_ptr)const
		{
			return intensity * 4 * M_PI;
		}
		scalar_t pdfImpl(const point_t&,const vector_t&)const;
	};
}

namespace ma{
	// PointLight Method Definitions
	template<typename Conf>
	PointLight<Conf>::PointLight(const typename Conf::transform_t &light2world,
		const typename Conf::spectrum_t &intens)
		: parent_type(light2world) ,intensity(intens){
			light_pos = ma::operator *(light2world , point_t(0,0,0));
	}
	template<typename Conf>
	typename Conf::spectrum_t PointLight<Conf>::sample_lImpl(const point_t &p, vector_t &wi,
		visibility_tester_t &visibility) const {
			wi = (light_pos - p);
			wi.normalize();
			visibility.setSegment(p, light_pos);
			return intensity /  (light_pos - p).squaredNorm();
	}
	template<typename Conf>
	typename Conf::spectrum_t PointLight<Conf>::sample_lImpl(const point_t &p, scalar_t u1,
		scalar_t u2, vector_t &wi, scalar_t& pdf,
		visibility_tester_t &visibility) const {
			pdf = 1;
			return sample_l(p, wi, visibility);
	}
	template<typename Conf>
	typename Conf::scalar_t PointLight<Conf>::pdfImpl(const point_t &, const vector_t &) const {
		return 0.;
	}
	template<typename Conf>
	typename Conf::spectrum_t
		PointLight<Conf>::sample_lImpl(const scene_ptr scene, scalar_t u1,
		scalar_t u2, scalar_t u3, scalar_t u4,
		ray_t &ray, scalar_t& pdf) const {
			ray.origin() = light_pos;
			ray.direction() = UniformSampleSphere(u1, u2);
			pdf = UniformSpherePdf<scalar_t>();
			return intensity;
	}

}
//extern "C" DLLEXPORT Light *CreateLight(const Transform &light2world,
//											const ParamSet &paramSet) {
//												Spectrum I = paramSet.FindOneSpectrum("I", Spectrum(1.0));
//												Point P = paramSet.FindOnePoint("from", Point(0,0,0));
//												Transform l2w = Translate(Vector(P.x, P.y, P.z)) * light2world;
//												return new PointLight(l2w, I);
//}

namespace ma{
	namespace details{
		template<typename Conf>
		struct light_creator<PointLight<Conf> >{
			typedef PointLight<Conf> light_t;
			typedef typename Conf::transform_t transform_t;
			typedef typename Conf::spectrum_t spectrum_t;
			typedef typename Conf::point_t point_t;
			typedef typename Conf::vector_t vector_t;
			light_t* operator()(const transform_t& light2world,const ParamSet& param_set)const{
				spectrum_t I = param_set.as<spectrum_t>("I", spectrum_t(1.0));
				point_t P = param_set.as<point_t>("from", point_t(0,0,0));
				transform_t l2w = transform_t().identity().translate(vector_t(P.x(), P.y(), P.z())) * light2world;
				return new light_t(l2w, I);
			}
		};
	}
}
#endif
