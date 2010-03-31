#include "modules.hpp"

namespace ma{


COREDLL shared_float_texture_t make_float_texture(const std::string& name,const transform_t& trans,const TextureParam& tp)
{
	return shared_float_texture_t();
}
COREDLL shared_spectrum_texture_t make_spectrum_texture(const std::string& name,const transform_t& trans,const TextureParam& tp)
{
	return shared_spectrum_texture_t();
}
}
