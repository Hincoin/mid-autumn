#ifndef _MA_INCLUDED_MATERIAL_GLASS_HPP_
#define _MA_INCLUDED_MATERIAL_GLASS_HPP_
#include "Material.hpp"
#include "Texture.hpp"
#include "TypeMap.hpp"

namespace ma{

	// Glass Class Declarations
	template<typename Conf>
	class Glass: public Material<Glass<Conf>,typename Conf::interface_config> {
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
		// Glass Public Methods
		Glass(texture_spectrum_ref kr,texture_spectrum_ref kt,
				texture_scalar_t_ref i,
			texture_scalar_t_ref bump) {
				Kr = kr;
				Kt = kt;
				index = i;
				bumpMap = bump;
		}
		bsdf_ptr getBSDFImpl(const differential_geometry_t &dgGeom,
			const differential_geometry_t &dgShading) const;
	private:
		// Glass Private Data
		texture_spectrum_ref Kr,Kt;
		texture_scalar_t_ref index;
		texture_scalar_t_ref bumpMap;
	};

	template<typename Conf>
		typename Glass<Conf>::bsdf_ptr
		Glass<Conf>::getBSDFImpl(const typename Glass<Conf>::differential_geometry_t &dgGeom,
				const typename Glass<Conf>::differential_geometry_t &dgShading)const
		{
			ADD_SAME_TYPEDEF(Conf,bsdf_t);
			ADD_SAME_TYPEDEF(Conf,specular_reflection_t);
			ADD_SAME_TYPEDEF(Conf,fresnel_dielectric_t);
			ADD_SAME_TYPEDEF(Conf,specular_transmission_t);
			differential_geometry_t dgs;
			if (bumpMap)
				bump(bumpMap, dgGeom, dgShading, ref(dgs));		
			else
				dgs = dgShading;
			bsdf_ptr bsdf ( new bsdf_t(dgs,dgGeom.normal));
			spectrum_t r = texture::evaluate(Kr,dgs).clamp();
			spectrum_t t = texture::evaluate(Kt,dgs).clamp();
			scalar_t ior = texture::evaluate(index,dgs);
			if(!r.black())
			{
				//printf("glass : not black\n");
				bsdf->add(new specular_reflection_t(r,new fresnel_dielectric_t(1,ior)));
			}
			if(!t.black())
				bsdf->add(new specular_transmission_t(t,1.,ior));
			return bsdf;
		}


}

namespace ma
{
	namespace details{
	
		template<typename Conf>
		struct material_creator<Glass<Conf> >{
			typedef Glass<Conf> glass_t;
			typedef typename Glass<Conf>::transform_t transform_t;

			ADD_SAME_TYPEDEF(Glass<Conf>,texture_spectrum_ref);
			ADD_SAME_TYPEDEF(Glass<Conf>,texture_scalar_t_ref);
			ADD_SAME_TYPEDEF(Conf,default_texture_spectrum_t);
			ADD_SAME_TYPEDEF(Conf,default_texture_scalar_t );
			ADD_SAME_TYPEDEF(Conf,spectrum_t);
			ADD_SAME_TYPEDEF(Conf,scalar_t);
			template<typename ParamT>
			Glass<Conf>* 
				operator()(const transform_t& xform,const ParamT& mp)const
			{
				texture_spectrum_ref Kr = (mp.getSpectrumTexture("Kr", 
					 (spectrum_t(1.f))) );
				texture_spectrum_ref Kt = mp.getSpectrumTexture("Kt",spectrum_t(1.f));
				assert(Kr);
				assert(Kt);
				texture_scalar_t_ref index = mp.getFloatTexture("index",1.5f);

				//texture_scalar_t_ref bumpMap = mp.getFloatTexture("bumpmap", 
				//	scalar_t(0.f) );
				//assert(bumpMap);
				texture_scalar_t_ref bumpMap = texture_scalar_t_ref();
				return new glass_t(Kr,Kt,index, bumpMap);
			}
		};
	}

	MAKE_TYPE_STR_MAP(1,Glass,glass)
	
}

#endif
