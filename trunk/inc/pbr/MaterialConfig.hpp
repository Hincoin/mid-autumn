#ifndef _MA_INCLUDED_MATERAIL_CONFIG_HPP_
#define _MA_INCLUDED_MATERAIL_CONFIG_HPP_

#include "BasicConfig.hpp"
#include "TextureConfig.hpp"

#include "Material.hpp"
#include "MaterialMatte.hpp"
#include "MaterialMirror.hpp"
#include "MaterialGlass.hpp"

//material configrations 
namespace ma
{

	template<typename B>
	struct material_interface_config:public B{
		typedef boost::shared_ptr<BSDF<bsdf_config<B> > > bsdf_ptr;

	typedef ConstantTexture<texture_config<B,typename B::spectrum_t> > const_texture_spectrum_t;
	typedef ConstantTexture<texture_config<B,typename B::scalar_t> > const_texture_scalar_t;
	typedef const_texture_spectrum_t default_texture_spectrum_t;
	typedef const_texture_scalar_t default_texture_scalar_t;
	
	typedef UVTexture<texture_config<B,typename B::spectrum_t> > uvtexture_spectrum_t;
	typedef CheckerBoard2D<checkerboard_texture_config<B,typename B::spectrum_t> > checkerboard_texture_spectrum_t;
	typedef CheckerBoard2D<checkerboard_texture_config<B,typename B::scalar_t> > checkerboard_texture_scalar_t;
	
	typedef boost::mpl::vector<const_texture_scalar_t,checkerboard_texture_scalar_t> float_texture_types;
	typedef boost::mpl::vector<const_texture_spectrum_t,uvtexture_spectrum_t,checkerboard_texture_spectrum_t> spectrum_texture_types;
	
	typedef typename make_shared_ptr_var_over_sequence<float_texture_types>::type texture_scalar_t_ref;
	typedef typename make_shared_ptr_var_over_sequence<spectrum_texture_types>::type texture_spectrum_ref;
	//typedef boost::shared_ptr<texture_spectrum_t>  texture_spectrum_ref;
	//typedef boost::shared_ptr<texture_scalar_t> texture_scalar_t_ref;


	typedef DifferentialGeometry<typename B::scalar_t,B::dimension> differential_geometry_t;
	};



	template<typename B> struct lambertian_bxdf_config;
	template<typename B> struct orennayar_bxdf_config;
	template<typename B>
	struct matte_material_config:public material_interface_config<B>{
		typedef material_interface_config<B> interface_config;
		//the following is matte material specific config
		typedef BSDF<bsdf_config<B> > bsdf_t;
		typedef Lambertian<lambertian_bxdf_config<B> > lambertian_t;
		typedef OrenNayar<orennayar_bxdf_config<B> > oren_nayar_t;
	};
	
	template<typename B> struct specular_reflection_config;
	template<typename B> struct specular_transmission_config;
	template<typename B> struct fresnel_types_generator;
	template<typename B>
	struct mirror_material_config:public material_interface_config<B>
	{
		typedef material_interface_config<B> interface_config;
		typedef BSDF<bsdf_config<B> > bsdf_t;

		typedef SpecularReflection<specular_reflection_config<B> > specular_reflection_t;
		typedef typename fresnel_types_generator<B>::fresnel_noop_t fresnel_noop_t;
	};
	template<typename B>
	struct glass_material_config:public material_interface_config<B>
	{
		typedef material_interface_config<B> interface_config;
		typedef BSDF<bsdf_config<B> > bsdf_t;

		typedef SpecularReflection<specular_reflection_config<B> > specular_reflection_t;
		typedef SpecularTransmission<specular_transmission_config<B> > specular_transmission_t;
		typedef typename fresnel_types_generator<B>::fresnel_dielectric_t fresnel_dielectric_t;
	};


	template<typename B>
	struct orennayar_bxdf_config:bxdf_config<B>{
		typedef bxdf_config<B> interface_config;	
	};
	template<typename B>
	struct lambertian_bxdf_config:bxdf_config<B>{
		typedef bxdf_config<B> interface_config;	
	}; 
	template<typename B>
		struct fresnel_config:B
		{
			typedef B interface_config;
		};
	template<typename B>
		struct fresnel_types_generator
		{
			typedef FresnelNoOp<fresnel_config<B> > fresnel_noop_t;
			typedef FresnelConductor<fresnel_config<B> > fresnel_conductor_t;
			typedef FresnelDielectric<fresnel_config<B> > fresnel_dielectric_t;
			typedef boost::mpl::vector<fresnel_noop_t,
					fresnel_conductor_t,
					fresnel_dielectric_t> fresnel_types;

			//typedef typename make_shared_ptr_var_over_sequence<fresnel_types>::type shared_fresnel_ptr;
			typedef typename make_ptr_var_over_sequence<fresnel_types>::type
				fresnel_ptr;
		};
	template<typename B>
	struct specular_reflection_config:bxdf_config<B>
	{
		typedef typename fresnel_types_generator<B>::fresnel_ptr fresnel_ptr;
		typedef bxdf_config<B> interface_config;
	};
	template<typename B>
		struct specular_transmission_config:bxdf_config<B>
	{
		typedef typename fresnel_types_generator<B>::fresnel_dielectric_t fresnel_dielectric_t;
		typedef bxdf_config<B> interface_config;
	};
}


#endif
