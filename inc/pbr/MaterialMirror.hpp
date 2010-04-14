#ifndef _MA_INCLUDED_MATERIALMIRROR_HPP_
#define _MA_INCLUDED_MATERIALMIRROR_HPP_
#include "Material.hpp"
#include "Texture.hpp"
#include "TypeMap.hpp"

namespace ma{

	// Mirror Class Declarations
	template<typename Conf>
	class Mirror: public Material<Mirror<Conf>,typename Conf::interface_config> {
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
		// Mirror Public Methods
		Mirror(texture_spectrum_ref kr,
			texture_scalar_t_ref bump) {
				Kr = kr;
				bumpMap = bump;
		}
		bsdf_ptr getBSDFImpl(const differential_geometry_t &dgGeom,
			const differential_geometry_t &dgShading) const;
	private:
		// Mirror Private Data
		texture_spectrum_ref Kr;
		texture_scalar_t_ref bumpMap;
	};

	template<typename Conf>
		typename Mirror<Conf>::bsdf_ptr
		Mirror<Conf>::getBSDFImpl(const typename Mirror<Conf>::differential_geometry_t &dgGeom,
				const typename Mirror<Conf>::differential_geometry_t &dgShading)const
		{
			ADD_SAME_TYPEDEF(Conf,bsdf_t);
			ADD_SAME_TYPEDEF(Conf,specular_reflection_t);
			ADD_SAME_TYPEDEF(Conf,fresnel_noop_t);
			differential_geometry_t dgs;
			if (bumpMap)
				bump(bumpMap, dgGeom, dgShading, ref(dgs));		
			else
				dgs = dgShading;
			bsdf_ptr bsdf ( new bsdf_t(dgs,dgGeom.normal));
			spectrum_t r = texture::evaluate(Kr,dgs).clamp();
			if(!r.black())
			{
				//printf("mirror : not black\n");
				bsdf->add(new specular_reflection_t(r,new fresnel_noop_t()));
			}
			return bsdf;
		}


}

namespace ma
{
	namespace details{
	
		template<typename Conf>
		struct material_creator<Mirror<Conf> >{
			typedef Mirror<Conf> mirror_t;
			typedef typename Mirror<Conf>::transform_t transform_t;

			ADD_SAME_TYPEDEF(Mirror<Conf>,texture_spectrum_ref);
			ADD_SAME_TYPEDEF(Mirror<Conf>,texture_scalar_t_ref);
			ADD_SAME_TYPEDEF(Conf,default_texture_spectrum_t);
			ADD_SAME_TYPEDEF(Conf,default_texture_scalar_t );
			ADD_SAME_TYPEDEF(Conf,spectrum_t);
			ADD_SAME_TYPEDEF(Conf,scalar_t);
			template<typename ParamT>
			Mirror<Conf>* 
				operator()(const transform_t& xform,const ParamT& mp)const
			{
				texture_spectrum_ref Kr = (mp.getSpectrumTexture("Kr", 
					 (spectrum_t(1.f))) );
				assert(Kr);

				//texture_scalar_t_ref bumpMap = mp.getFloatTexture("bumpmap", 
				//	scalar_t(0.f) );
				//assert(bumpMap);
				texture_scalar_t_ref bumpMap = texture_scalar_t_ref();
				return new mirror_t(Kr, bumpMap);
			}
		};
	}

	MAKE_TYPE_STR_MAP(1,Mirror,mirror)
	
}
#endif
