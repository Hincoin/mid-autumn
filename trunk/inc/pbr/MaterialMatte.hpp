#ifndef _MA_INCLUDED_MATERIALMATTE_HPP_
#define _MA_INCLUDED_MATERIALMATTE_HPP_

#include "Material.hpp"
#include "Texture.hpp"

namespace ma{

	// Matte Class Declarations
	template<typename Conf>
	class Matte : public Material<Matte<Conf>,Conf::interface_config> {
	public:
		ADD_SAME_TYPEDEF(Conf,bsdf_ptr);

		ADD_SAME_TYPEDEF(Conf,default_texture_spectrum_t);
		ADD_SAME_TYPEDEF(Conf,default_texture_scalar_t );

		ADD_SAME_TYPEDEF(Conf,texture_spectrum_ref);
		ADD_SAME_TYPEDEF(Conf,texture_scalar_t_ref);
		ADD_SAME_TYPEDEF(Conf,differential_geometry_t);
		ADD_SAME_TYPEDEF(Conf,spectrum_t);
		ADD_SAME_TYPEDEF(Conf,scalar_t);
	public:
		// Matte Public Methods
		Matte(texture_spectrum_ref kd,
			texture_scalar_t_ref sig,
			texture_scalar_t_ref bump) {
				Kd = kd;
				sigma = sig;
				bumpMap = bump;
		}
		bsdf_ptr getBSDFImpl(const differential_geometry_t &dgGeom,
			const differential_geometry_t &dgShading) const;
	private:
		// Matte Private Data
		texture_spectrum_ref Kd;
		texture_scalar_t_ref sigma, bumpMap;
	};
	// Matte Method Definitions
	template<typename Conf>
	typename Conf::bsdf_ptr 
		Matte<Conf>::getBSDFImpl(const typename Conf::differential_geometry_t &dgGeom,
		const typename Conf::differential_geometry_t &dgShading) const {
			ADD_SAME_TYPEDEF(Conf,lambertian_t);
			ADD_SAME_TYPEDEF(Conf,oren_nayar_t);
			ADD_SAME_TYPEDEF(Conf,bsdf_t);
			// Allocate _BSDF_, possibly doing bump-mapping with _bumpMap_
			differential_geometry_t dgs;
			if (bumpMap)
				bump(bumpMap, dgGeom, dgShading, dgs);
			else
				dgs = dgShading;
			//bsdf_ptr bsdf = BSDF_ALLOC(bsdf_t)(dgs, dgGeom.normal);
			//// Evaluate textures for _Matte_ material and allocate BRDF
			//spectrum_t r = Kd->evaluate(dgs).Clamp();
			//scalar_t sig = ma::clamp(sigma->evaluate(dgs), scalar_t(0), scalar_t(90));
			//if (sig == scalar_t(0))
			//	bsdf->add(BSDF_ALLOC(lambertian_t)(r));
			//else
			//	bsdf->add(BSDF_ALLOC(oren_nayar_t)(r, sig));
			//return bsdf;

			//memory leak here change to be a smart-ptr
			bsdf_ptr bsdf(new bsdf_t(dgs, dgGeom.normal));
			// Evaluate textures for _Matte_ material and allocate BRDF
			spectrum_t r = texture::evaluate(Kd,dgs).Clamp();
			scalar_t sig = ma::clamp( texture::evaluate(sigma,dgs), scalar_t(0), scalar_t(90));
			if (sig == scalar_t(0))
				bsdf->add(new lambertian_t(r));
			else
				bsdf->add(new oren_nayar_t(r, sig));//change to ptr_var
			return bsdf;
	}
}

namespace ma{
	namespace details{
		template<typename Conf>
		struct material_creator<Matte<Conf> >{
			typedef Matte<Conf> matte_t;
			typedef typename Matte<Conf>::transform_t transform_t;

			ADD_SAME_TYPEDEF(Matte<Conf>,texture_spectrum_ref);
			ADD_SAME_TYPEDEF(Matte<Conf>,texture_scalar_t_ref);
			ADD_SAME_TYPEDEF(Conf,default_texture_spectrum_t);
			ADD_SAME_TYPEDEF(Conf,default_texture_scalar_t );
			ADD_SAME_TYPEDEF(Conf,spectrum_t);
			ADD_SAME_TYPEDEF(Conf,scalar_t);
			Matte<Conf>* 
				operator()(const transform_t& xform,const ParamSet& mp)const
			{
				texture_spectrum_ref Kd = mp.as<texture_spectrum_ref>("Kd", 
					texture_spectrum_ref (new default_texture_spectrum_t (spectrum_t(1.f))) );

				texture_scalar_t_ref sigma = mp.as<texture_scalar_t_ref>("sigma",
					texture_scalar_t_ref (new default_texture_scalar_t (scalar_t(0.f))) );
				texture_scalar_t_ref bumpMap = mp.as<texture_scalar_t_ref>("bumpmap", 
					texture_scalar_t_ref () );
				return new matte_t(Kd, sigma, bumpMap);
			}
		};
	}
}
#endif
