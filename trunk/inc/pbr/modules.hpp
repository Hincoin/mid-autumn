#ifndef _MA_INCLUDED_DEFAULTCONFIGURATIONS_HPP_
#define _MA_INCLUDED_DEFAULTCONFIGURATIONS_HPP_
#include <string>
#include "DefaultConfigurations.hpp"
#include "TextureParam.hpp"


namespace ma{
COREDLL shared_float_texture_t make_float_texture(const std::string& name,const transform_t& trans,const TextureParam& tp);
COREDLL shared_spectrum_texture_t make_spectrum_texture(const std::string& name,const transform_t& trans,const TextureParam& tp);
//
//
using std::string;
COREDLL shape_ref_t make_shape(const string& name,const transform_t& obj2world, bool reverse_orientation,const ParamSet& param);
COREDLL material_ref_t make_material(const string& name,const transform_t &mtl2world,const TextureParam& mp);
COREDLL light_ptr make_light(const string& name,const transform_t& light2world,const ParamSet& param);
COREDLL surface_integrator_ptr make_surface_integrator(const string& name,const ParamSet& param);
COREDLL primitive_ptr make_accelerator(const string& name,
		const std::vector<primitive_ref_t>& prims,
		const ParamSet& param);
COREDLL camera_ptr make_camera(const string& name,
		const ParamSet &param,const transform_t& world2cam,film_ptr film);

COREDLL sampler_ptr make_sampler(const string& name,
		const ParamSet &param,const film_ptr film);
COREDLL filter_ptr make_filter(const string& name,
		const ParamSet &param);
COREDLL film_ptr make_film(const string& name,
		const ParamSet &param,filter_ptr filt);
}
namespace ma{


typedef shared_float_texture_t (*create_float_texture_func_t)(const transform_t&,const TextureParam&);
typedef shared_spectrum_texture_t (*create_spectrum_texture_func_t)(const transform_t&,const TextureParam&);
typedef shape_ref_t (*create_shape_func_t)(const transform_t&,bool,const ParamSet&);
typedef material_ref_t (*create_material_func_t)(const transform_t&,const TextureParam&);
typedef light_ptr (*create_light_func_t)(const transform_t&,const ParamSet&);
typedef surface_integrator_ptr (*create_surface_integrator_func_t)(const ParamSet&);
typedef primitive_ptr (*create_accelerator_func_t)(const std::vector<primitive_ref_t>&,const ParamSet&);
typedef camera_ptr (*create_camera_func_t)(const ParamSet&,const transform_t&,film_ptr);
typedef sampler_ptr (*create_sampler_func_t)(const ParamSet&,const film_ptr);
typedef filter_ptr (*create_filter_func_t)(const ParamSet&);
typedef film_ptr (*create_film_func_t)(const ParamSet&,const filter_ptr);

template<typename FunT,typename MapT>
	void register_creator(const std::string& name,FunT f,MapT& m)
	{
		m.insert(std::make_pair(name,f));
	}
void register_all_creators();
}
#endif



