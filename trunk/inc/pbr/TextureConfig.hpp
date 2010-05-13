#ifndef _MA_INCLUDED_TEXTURE_CONFIG_HPP_
#define _MA_INCLUDED_TEXTURE_CONFIG_HPP_

#include "Texture.hpp"
#include "TextureUV.hpp"
#include "TextureCheckerBoard.hpp"
#include "BasicConfig.hpp"
namespace ma
{
											

	template<typename B>
	struct texturemapping_config:B{
		typedef B interface_config;	
	};	
	template<typename B,typename S>
	struct texture_interface_config:B {
		typedef S color_t;
	};

	//specific
	template<typename B,typename S>
	struct texture_config:texture_interface_config<B,S> {
		typedef texture_interface_config<B,S> interface_config; 
		typedef SphericalMapping2D<texturemapping_config<B> > spherical_mapping2d_t;
		typedef CylindricalMapping2D<texturemapping_config<B> > cylindrical_mapping2d_t;
		typedef PlanarMapping2D<texturemapping_config<B> > planar_mapping2d_t;
		typedef UVMapping2D<texturemapping_config<B> > uv_mapping2d_t;
		typedef ptr_var<spherical_mapping2d_t,cylindrical_mapping2d_t,planar_mapping2d_t,uv_mapping2d_t> texturemap2d_ptr;
	};
	template<typename B,typename S>
		struct checkerboard_texture_config;

	template<typename B,typename S>
		struct checkerboard_texture_config:texture_config<B,S>
	{
		typedef typename boost::mpl::if_<boost::is_same<S,typename B::spectrum_t>,
			typename material_interface_config<B>::texture_spectrum_ref
				,
			typename material_interface_config<B>::texture_scalar_t_ref
			>::type texture_ref_t; 
	};


}

#endif
