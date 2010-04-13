#include "TextureParam.hpp"

namespace ma{
using std::string;
		shared_spectrum_texture_t TextureParam::getSpectrumTexture(const std::string& name,
		const spectrum_t& def)const
{
	string tname = geom_params_.as<string>(name);
	if (tname == "") tname = material_params_.as<string>(name);
	if (tname != "")
	{
		std::map<string,shared_spectrum_texture_t>::const_iterator it = spectrum_textures_.find(tname);
		if (it != spectrum_textures_.end())
		{
			assert(spectrum_textures_[tname]);
			return it->second;	
		}
		else
			report_warning("Couldn't find spectrum texture named \"%s\"",name.c_str());
	}
	spectrum_t val = geom_params_.as<spectrum_t>(name,material_params_.as<spectrum_t>(name,def));
	return shared_spectrum_texture_t(new default_spectrum_texture_t(val)); 
}
		shared_float_texture_t TextureParam::getFloatTexture(const std::string& name,
				float def)const
{
	string tname = geom_params_.as<string>(name);
	if (tname == "") tname = material_params_.as<string>(name);
	if (tname != "")
	{
		std::map<string,shared_float_texture_t>::const_iterator it = float_textures_.find(tname);
		if (it != float_textures_.end())
		{
			assert(float_textures_[tname]);
			return it->second;	
		}
		else
			report_warning("Couldn't find spectrum texture named \"%s\"",name.c_str());
	}
	float val = geom_params_.as<float>(name,material_params_.as<float>(name,def));
	return shared_float_texture_t(new default_float_texture_t(val)); 

}
}
