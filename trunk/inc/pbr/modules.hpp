#ifndef _MA_INCLUDED_DEFAULTCONFIGURATIONS_HPP_
#define _MA_INCLUDED_DEFAULTCONFIGURATIONS_HPP_
#include "DefaultConfigurations.hpp"
#include "TextureParam.hpp"


namespace ma{
COREDLL shared_float_texture_t make_float_texture(const std::string& name,const transform_t& trans,const TextureParam& tp);
COREDLL shared_spectrum_texture_t make_spectrum_texture(const std::string& name,const transform_t& trans,const TextureParam& tp);
}
#endif



