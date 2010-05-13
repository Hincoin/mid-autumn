#ifndef _MA_INCLUDED_TEXTUREPARAM_HPP_
#define _MA_INCLUDED_TEXTUREPARAM_HPP_

#include <map>

#include "ParamSet.hpp"
#include "DefaultConfigurations.hpp"
namespace ma{
class TextureParam{
	public:
		TextureParam(const ParamSet& geomp,const ParamSet& matp,
				const std::map<std::string,shared_float_texture_t>& ft,
				const std::map<std::string,shared_spectrum_texture_t>& st)
			:geom_params_(geomp),material_params_(matp),
			float_textures_(ft),
			spectrum_textures_(st){}
		shared_spectrum_texture_t getSpectrumTexture(const std::string& name,
		const spectrum_t& def)const;
		shared_float_texture_t getFloatTexture(const std::string& name,
				float def)const;
		template<typename T>
		T	as(const std::string& name,const T& d)const
			{ 
				return geom_params_.as<T>(name,material_params_.as<T>(name,d));
			}
		const ParamSet& getGeomParams()const{return geom_params_;}
		const ParamSet& getMaterialParams()const{return material_params_;}
	private:
		const ParamSet & geom_params_,&material_params_;
	const std::map<std::string,shared_float_texture_t>& float_textures_;
	const std::map<std::string,shared_spectrum_texture_t>& spectrum_textures_;
};
}
#endif
