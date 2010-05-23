#include "boost/mpl/for_each.hpp"
#include "modules.hpp"


#include "KdTree.hpp"
namespace ma{

using std::string;
//builtin types
static std::map<string,create_float_texture_func_t> float_texture_creators;
static std::map<string,create_spectrum_texture_func_t> spectrum_texture_creators;
static std::map<string,create_shape_func_t> shape_creators;
static std::map<string,create_material_func_t> material_creators;
static std::map<string,create_light_func_t> light_creators;
static std::map<string,create_surface_integrator_func_t> surface_integrator_creators;
static std::map<string,create_accelerator_func_t> accelerator_creators;
static std::map<string,create_camera_func_t> camera_creators;
static std::map<string,create_sampler_func_t> sampler_creators;
static std::map<string,create_filter_func_t> filter_creators;
static std::map<string,create_film_func_t> film_creators;

COREDLL shared_float_texture_t make_float_texture(const std::string& name,const transform_t& trans,const TextureParam& tp)
{
	std::map<string,create_float_texture_func_t>::const_iterator it =
		float_texture_creators.find(name);
	if (it != float_texture_creators.end())
		return (*it->second)(trans,tp);
	//load plugin and create
	return shared_float_texture_t();
}
COREDLL shared_spectrum_texture_t make_spectrum_texture(const std::string& name,const transform_t& trans,const TextureParam& tp)
{
	std::map<string,create_spectrum_texture_func_t>::const_iterator it =
		spectrum_texture_creators.find(name);
	if (it != spectrum_texture_creators.end())
		return (*it->second)(trans,tp);
	
	return shared_spectrum_texture_t();
}
//
//
COREDLL shape_ref_t make_shape(const string& name,const transform_t& obj2world, bool reverse_orientation,const ParamSet& param)
{
	std::map<string,create_shape_func_t>::const_iterator it = shape_creators.find(name);
	if (it != shape_creators.end())
		return (*it->second)(obj2world,reverse_orientation,param);

	return shape_ref_t();
}
COREDLL material_ref_t make_material(const string& name,const transform_t &mtl2world,const TextureParam& mp)
{
	std::map<string,create_material_func_t>::const_iterator it = material_creators.find(name);
	if (it != material_creators.end())
		return (*it->second)(mtl2world,mp);

	return material_ref_t();
	//return material_ref_t(create_material<material_t>(mtl2world,mp.getMaterialParams()));
}
COREDLL light_ptr make_light(const string& name,const transform_t& light2world,const ParamSet& param)
{
//	return light_ptr();
	std::map<string,create_light_func_t>::const_iterator it = light_creators.find(name);
	if (it != light_creators.end())
		return (*it->second)(light2world,param);
	return light_ptr();
}
COREDLL surface_integrator_ptr make_surface_integrator(const string& name,const ParamSet& param)
{
	 ////////////////////////////////////////////////////////////////////////////
	std::map<string,create_surface_integrator_func_t>::const_iterator it = surface_integrator_creators.find(name);
	if (it != surface_integrator_creators.end())
		return (*it->second)(param);
	return surface_integrator_ptr();
}
COREDLL primitive_ptr make_accelerator(const string& name,
		const std::vector<primitive_ref_t>& prims,
		const ParamSet& param)
{
	std::map<string,create_accelerator_func_t>::const_iterator it = accelerator_creators.find(name);
	if (it != accelerator_creators.end())
		return (*it->second)(prims,param);
	return primitive_ptr();
}

COREDLL sampler_ptr make_sampler(const string& name,
		const ParamSet &param,const film_ptr film)
{
	std::map<string,create_sampler_func_t>::const_iterator it = sampler_creators.find(name);
	if(it != sampler_creators.end())
		return (*it->second)(param,film);
	return sampler_ptr();
}
COREDLL filter_ptr make_filter(const string& name,
		const ParamSet &param)
{
	std::map<string,create_filter_func_t>::const_iterator it = filter_creators.find(name);
	if(it != filter_creators.end())
		return (*it->second)(param);
	return filter_ptr();
}
COREDLL film_ptr make_film(const string& name,
		const ParamSet &param,filter_ptr filt)
{
	std::map<string,create_film_func_t>::const_iterator it = film_creators.find(name);
	if(it != film_creators.end())
		return (*it->second)(param,filt);
	return film_ptr();	
}
COREDLL camera_ptr make_camera(const string& name,
		const ParamSet& param,const transform_t& xform,film_ptr film)
{
	std::map<string,create_camera_func_t>::const_iterator it = camera_creators.find(name);
	if (it != camera_creators.end())
		return (*it->second)(param,xform,film);
	return camera_ptr();
}
////////////////////////////////////////////////////////////////////////////////////
/*
*/
////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
template<typename M>
material_ref_t material_creator_func_wrapper(const transform_t& xf,const TextureParam& mp)
{
	return material_ref_t(create_material<M>(xf,mp));
}
struct register_material_func{
template<typename T>
void operator()(T&)const
{
	register_creator(map_type_str<typename T::type>::type_str(),&material_creator_func_wrapper<typename T::type>,material_creators);
}
};
////////////////////////////////////////////////////////////////////////////////////////////
template<typename C>
camera_ptr camera_creator_func_wrapper(const ParamSet& param,const transform_t& xform,film_ptr film)
{
	return create_camera<C>(param,xform,film);
}
struct register_camera_func{
template<typename C>
	void operator()(C&)const
	{
		typedef typename C::type camera_type;
		register_creator(map_type_str<camera_type>::type_str(),&camera_creator_func_wrapper<camera_type>,camera_creators);
	}
};
/////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
shared_float_texture_t float_texture_creator_func_wrapper(const transform_t& xform,const TextureParam& tp)
{
	return shared_float_texture_t(create_texture<T>(xform,tp));
}
struct register_float_texture_func{
template<typename T>
	void operator()(T&)const
	{
		typedef typename T::type texture_type;
		register_creator(map_type_str<texture_type>::type_str(),&float_texture_creator_func_wrapper<texture_type>,float_texture_creators);
	}
};
//////////////////////////////////////////////////////////////////////////////////
template<typename T>
shared_spectrum_texture_t spectrum_texture_creator_func_wrapper(const transform_t& xform,const TextureParam& tp)
{
	return shared_spectrum_texture_t(create_texture<T>(xform,tp));
}
struct register_spectrum_texture_func{
template<typename T>
	void operator()(T&)const
	{
		typedef typename T::type texture_type;
		register_creator(map_type_str<texture_type>::type_str(),&spectrum_texture_creator_func_wrapper<texture_type>,spectrum_texture_creators);
	}
};
///////////////////////////////////////////////////////////////////////////////////
template<typename S>
shape_ref_t shape_creator_func_wrapper(const transform_t& xform,bool reverse,const ParamSet& param)
{
	return shape_ref_t(create_shape<S>(xform,reverse,param));
}
struct register_shape_func{
template<typename S>
	void operator()(S&)const
	{
		typedef typename S::type shape_type;
		register_creator(map_type_str<shape_type>::type_str(),&shape_creator_func_wrapper<shape_type>,
				shape_creators);
	}
};
////////////////////////////////////////////////////////////////////////////////////
//
template<typename L>
light_ptr light_creator_func_wrapper(const transform_t& xform,const ParamSet& param)
{
	return light_ptr(create_light<L>(xform,param));
}
struct register_light_func{
template<typename L>
	void operator()(L&)const
	{
		typedef typename L::type light_type;
		register_creator(map_type_str<light_type>::type_str(),&light_creator_func_wrapper<light_type>,
				light_creators);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////
template<typename SI>
surface_integrator_ptr surface_integrator_creator_func_wrapper(const ParamSet& param)
{return surface_integrator_ptr(create_integrator<SI>(param));}
struct register_surface_integrator_func{
template<typename SI>
	void operator()(SI&)const
	{
		typedef typename SI::type surface_integrator_type;
		register_creator(map_type_str<surface_integrator_type>::type_str(),
				&surface_integrator_creator_func_wrapper<surface_integrator_type>,
				surface_integrator_creators);
	}
};
//////////////////////////////////////////////////////////////////////////////////////////
template<typename A>
primitive_ptr accelerator_creator_func_wrapper(const std::vector<primitive_ref_t>& prims,const ParamSet& param)
{
	return primitive_ptr(create_accelerator<A>(prims,param));
}
struct register_accelerator_func{
template<typename A>
	void operator()(A&)const
	{
		typedef typename A::type accelerator_type;
		register_creator(map_type_str<accelerator_type>::type_str(),
				&accelerator_creator_func_wrapper<accelerator_type>,
				accelerator_creators);
	}

};
//
template<typename S>
sampler_ptr sampler_creator_func_wrapper(const ParamSet& param,const film_ptr film)
{
	return sampler_ptr(create_sampler<S>(param,film));
}
struct register_sampler_func{
template<typename S>
	void operator()(S&)const
	{
		typedef typename S::type sampler_type;
		register_creator(map_type_str<sampler_type>::type_str(),
				&sampler_creator_func_wrapper<sampler_type>,
				sampler_creators);
	}

};
//
//
//
template<typename F>
filter_ptr filter_creator_func_wrapper(const ParamSet& param)
{
	return filter_ptr(create_filter<F>(param));
}
struct register_filter_func{
template<typename F>
	void operator()(F&)const
	{
		typedef typename F::type filter_type;
		register_creator(map_type_str<filter_type>::type_str(),
				&filter_creator_func_wrapper<filter_type>,
				filter_creators);
	}

};


void register_all_creators()
{
#define REGISTER_ALL_TYPES(Types,F)\
	boost::mpl::for_each<boost::mpl::transform<Types,identity_wrapper<boost::mpl::_1> >::type>(F());\


	typedef boost::mpl::vector<KdTreeAccel> accelerator_types;

	REGISTER_ALL_TYPES(material_types,register_material_func)
	REGISTER_ALL_TYPES(camera_types,register_camera_func)
	REGISTER_ALL_TYPES(float_texture_types,register_float_texture_func)
	REGISTER_ALL_TYPES(spectrum_texture_types,register_spectrum_texture_func)
	REGISTER_ALL_TYPES(shape_types,register_shape_func)
	REGISTER_ALL_TYPES(light_types,register_light_func)
	REGISTER_ALL_TYPES(surface_integrator_types,register_surface_integrator_func)
	REGISTER_ALL_TYPES(accelerator_types,register_accelerator_func)
	REGISTER_ALL_TYPES(sampler_types,register_sampler_func)
	REGISTER_ALL_TYPES(film_types,register_film_func)
	REGISTER_ALL_TYPES(filter_types,register_filter_func)
}
struct release_memory_func{
template<typename F>
	void operator()(F&)const
	{
		typedef typename F::type type;
		type::release_memory();
	}

};

void release_memory()
{
#define RELEASE_MEMORY_TYPES(Types,F)\
	boost::mpl::for_each<boost::mpl::transform<Types,identity_wrapper<boost::mpl::_1> >::type>(F());\

	bsdf_t::release_memory();
	/*bool is_released = */
	//assert(is_released);
	//release bxdf_types
	RELEASE_MEMORY_TYPES(bxdf_types,release_memory_func)
}

	void register_film_creator(const std::string& name,create_film_func_t fp)
	{
			register_creator(name,
				fp,
				film_creators);
	}
}
