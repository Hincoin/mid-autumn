#ifndef _MA_INCLUDED_DEFAULT_CONFIGURATIONS_HPP_
#define _MA_INCLUDED_DEFAULT_CONFIGURATIONS_HPP_

#include "Vector.hpp"
#include "Ray.hpp"

namespace ma{



}
#include <boost/mpl/vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "Camera.hpp"
#include "Texture.hpp"
#include "Scene.hpp"
#include "Color.hpp"
#include "DifferentialGeometry.hpp"
#include "Film.hpp"
#include "Geometry.hpp"
#include "Intersect.hpp"
#include "Light.hpp"
#include "Material.hpp"
#include "Primitive.hpp"
#include "Ray.hpp"
#include "Reflection.hpp"
#include "Sampling.hpp"
#include "Scene.hpp"
#include "Shape.hpp"
#include "Texture.hpp"
#include "Transport.hpp"
#include "Transform.hpp"
//implementations
#include "ImageFilm.hpp"
#include "LDSampler.hpp"
#include "MaterialMatte.hpp"
#include "PerspectiveCamera.hpp"
#include "PointLight.hpp"
#include "TextureUV.hpp"
#include "TextureCheckerBoard.hpp"
#include "WhittedIntegrator.hpp"
#include "TriangleMesh.hpp"
#include "Mitchell.hpp"
#include "MaterialMirror.hpp"
#include "MaterialGlass.hpp"

namespace ma{
	template<typename T=float,int D=3>
	struct basic_config{
		typedef T scalar_t;
		static const int dimension = D;

		typedef typename vector_type<T,D>::type vector_t;
		typedef typename matrix_type<T,D,D>::type matrix_t;
		typedef typename transform_type<T,D>::type transform_t;

		typedef typename ray_type<T,D>::type ray_t;
		typedef typename point_type<T,D>::type point_t;
		typedef typename normal_type<T,D>::type normal_t;
		typedef SpaceSegment<vector_t> bbox_t;

		static const int color_sample = 3;
		typedef Spectrum< scalar_t,color_sample> spectrum_t;
		//////////////////////////////////////////////////////////////////////////
		//temp typedef
		//typedef ray_t ray_differental_t;

		typedef RayDifferential<vector_t> ray_differential_t;

		typedef DifferentialGeometry<scalar_t,D> differential_geometry_t;

	};
	//declarations
	template<typename B>
	struct film_config;
	template<typename B>
	struct camera_config;
	template<typename B>
	struct surface_integrator_config;
	template<typename B>
	struct volume_integrator_config ;
	template<typename B>
	struct sampler_config;
	template<typename B>
	struct sample_config;

	template<typename B>
	struct primitive_config;
	template<typename B>
	struct geometry_primitive_config;

	template<typename B>
	struct light_config;
	template<typename B>
	struct volume_region_config;
	template<typename B>
	struct intersection_config;
	template<typename B>
	struct bbox_config;
	template<typename B>
	struct scene_config;
	template<typename B>
	struct visibility_tester_config;
	template<typename B>
	struct bsdf_config;
	template<typename B>
	struct material_config;
	template<typename B,typename S>
	struct texture_config;
	template<typename B>
	struct bxdf_config;
	template<typename B>
	struct filter_config;
	//////////////////////////////////////////////////////////////////////////
	template<typename B>
	struct filter_config:public B{};
	template<typename B>
	struct mitchellfilter_config:filter_config<B>{
		typedef filter_config<B> interface_config;
	};
	template<typename B>
	struct film_config:B{
		typedef CameraSample<sample_config<B> > sample_t;
///////////////////////////////////////////////////////////////////////////
//subclass needed
		typedef MitchellFilter<mitchellfilter_config<B> > filter_t;
		typedef boost::mpl::vector<filter_t> filter_types;
		typedef filter_t* filter_ptr;
	};
	template<typename B>
	struct image_film_config:film_config<B>{
		typedef film_config<B> interface_config;
	};
	template<typename B>
	struct camera_config:B{
		typedef CameraSample<sample_config<B> > sample_t;
		typedef ImageFilm<image_film_config<B> > film_t;
		typedef boost::mpl::vector<film_t> film_types;
		typedef film_t* film_ptr;
		//ADD_CRTP_INTERFACE_TYPEDEF(film_ptr);
	};
	template <typename B>
	struct perspective_camera_config:camera_config<B>{
		typedef camera_config<B> interface_config;	
	};
	template<typename B>
	struct surface_integrator_config:B{
		typedef Scene<scene_config<B> >* scene_ptr;
		//typedef Sample<sample_config<B> >* sample_ptr;
		typedef typename scene_config<B>::sample_ptr sample_ptr;
		//typedef RayDifferential<typename B::vector_t> ray_differential_t;
		typedef typename scene_config<B>::ray_differential_t ray_differential_t;

		typedef IntegratorSample<sample_config<B> > sample_t;
		//typedef Intersection<intersection_config<B> > intersection_t;
		typedef typename scene_config<B>::intersection_t intersection_t;


		typedef boost::shared_ptr<BSDF<bsdf_config<B> > > bsdf_ptr;
		typedef VisibilityTester<visibility_tester_config<B> > visibility_tester_t;
	};
	template<typename B>
	struct volume_integrator_config:B{};
	template<typename B>
	struct sampler_config:B{
		typedef typename scene_config<B>::sample_t sample_t;
	};
	template<typename B>
	struct ldsampler_config:sampler_config<B>{
		typedef sampler_config<B> interface_config;
	};
	template<typename B>
	struct sample_config:B{
		//typedef Scene<scene_config<B> >* scene_ptr;
		//typedef WhittedIntegrator<surface_integrator_config<B> >* surface_integrator_ptr;
		//ADD_SAME_TYPEDEF(Conf,surface_integrator_ptr);
		//ADD_SAME_TYPEDEF(Conf,volume_integrator_ptr);
		//typedef void* volume_integrator_ptr;
	};

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
	template<typename B,typename S>
		struct checkerboard_texture_config:texture_config<B,S>
	{
		typedef typename boost::mpl::if_<boost::is_same<S,typename B::spectrum_t>,
			typename material_interface_config<B>::texture_spectrum_ref
				,
			typename material_interface_config<B>::texture_scalar_t_ref
			>::type texture_ref_t; 
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
	struct primitive_interface_config:B{
		typedef primitive_interface_config<B> interface_config;

		typedef typename B::scalar_t ScalarType;
		static const int Dimension = B::dimension;


		//////////////////////////////////////////////////////////////////////////
		//typedef Intersection<intersection_config<B> > intersection_t;
		typedef typename scene_config<B>::intersection_t intersection_t;
		typedef boost::shared_ptr<BSDF<bsdf_config<B> > > bsdf_ptr;
		typedef DifferentialGeometry<typename B::scalar_t,B::dimension> differential_geometry_t;
	};
	template<typename B>
	struct refined_primitive_config;
	template<typename B>
	struct primitive_config:primitive_interface_config<B>{
		typedef boost::mpl::vector<
			Matte<matte_material_config<B> >
			,Mirror<mirror_material_config<B> >
			,Glass<glass_material_config<B> >
			> material_types;
	};
	template<typename B>
	struct geometry_primitive_config:primitive_config<B>{
	    typedef primitive_config<B> parent_type;
		typedef MATriangleMesh<short_index_tri_mesh_cfg> shape_t;//65535 vertex supported
		typedef boost::mpl::vector<shape_t> shape_types;
		typedef MAGeometryPrimitive<refined_primitive_config<B> > refined_primitive_type;
		typedef MATriangle<default_tri_cfg<shape_t> > refined_shape_t;
		typedef boost::shared_ptr<refined_shape_t> refined_shape_ref_t;


		typedef boost::shared_ptr<shape_t> shape_ref_t;
		//typedef boost::shared_ptr<typename parent_type::material_t> material_ref_t;
		typedef typename make_shared_ptr_var_over_sequence<typename parent_type::material_types>::type material_ref_t;
	
		typedef const boost::shared_ptr<shape_t> const_shape_ref_t;
		typedef material_ref_t const const_material_ref_t;
	};
	template<typename B>
	struct refined_primitive_config:geometry_primitive_config<B>{
		typedef typename geometry_primitive_config<B>::refined_shape_t shape_t;
		typedef const boost::shared_ptr<shape_t> const_shape_ref_t;
	};
	template<typename B>
	struct visibility_tester_config:B{
		typedef Scene<scene_config<B> >* scene_ptr;
	};
	template<typename B>
	struct light_config:public B{
		typedef Scene<scene_config<B> >* scene_ptr;
		typedef VisibilityTester<visibility_tester_config<B> > visibility_tester_t;
		//typedef RayDifferential<typename B::vector_t> ray_differential_t;
		typedef typename scene_config<B>::ray_differential_t ray_differential_t;
	};
	template<typename B>
	struct point_light_config:light_config<B>{
	typedef light_config<B> interface_config;	
	};
	template<typename B>
	struct volume_region_config:B{};

	template<typename B>
	struct bxdf_config:public B{

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
	template<typename B>
	struct bsdf_config:public B{
		typedef DifferentialGeometry<typename B::scalar_t,B::dimension> differential_geometry_t;
		//typedef Lambertian<bxdf_config<B> >* BxDF_ptr;
		typedef ptr_var<
		Lambertian<lambertian_bxdf_config<B> >,
			OrenNayar<orennayar_bxdf_config<B> >,
		SpecularReflection<specular_reflection_config<B> >,
		SpecularTransmission<specular_transmission_config<B> >
			   	> BxDF_ptr;
	};
	template<typename B>
	struct intersection_config:B{
		//typedef const MAPrimitive< primitive_interface_config<B> >* primitive_ptr;
		typedef typename pointer_to_const<typename scene_config<B>::primitive_ptr>::type primitive_ptr;
		//typedef RayDifferential<typename B::vector_t> ray_differential_t;
		typedef typename scene_config<B>::ray_differential_t ray_differential_t;
		typedef DifferentialGeometry<typename B::scalar_t,B::dimension> differential_geometry_t;
		typedef boost::shared_ptr<BSDF<bsdf_config<B> > > bsdf_ptr;
	};

	//light , camera, surface_integrator, sampler, primitive, 
	//volume_integrator 
template<typename B>
struct scene_config:B{
	typedef Intersection<intersection_config<B> > intersection_t;
	//typedef typename intersection_config<B>::ray_differential_t ray_differential_t;
	typedef void* volume_integrator_ptr;
	typedef void* volume_region_ptr;

		//ADD_SAME_TYPEDEF(Conf,volume_integrator_ptr)
		//ADD_SAME_TYPEDEF(Conf,volume_region_ptr)
		//ADD_SAME_TYPEDEF(Conf,intersection_t)
	//type array
	typedef boost::mpl::vector<PerspectiveCamera<perspective_camera_config<B> > > camera_types;
	typedef boost::mpl::vector<PointLight<point_light_config<B> > > light_types;
	typedef boost::mpl::vector<WhittedIntegrator<surface_integrator_config<B> > > surface_integrator_types;
	typedef boost::mpl::vector<LDSampler<ldsampler_config<B> > > sampler_types;
	typedef Sample<sample_config<B> > sample_t;
	typedef sample_t* sample_ptr;

	typedef typename make_ptr_var_over_sequence<camera_types>::type camera_ptr;
	typedef typename make_ptr_var_over_sequence<light_types>::type light_ptr;
	typedef typename make_ptr_var_over_sequence<surface_integrator_types>::type surface_integrator_ptr;
	typedef typename make_ptr_var_over_sequence<sampler_types>::type sampler_ptr;
	typedef MAPrimitive<primitive_interface_config<B> >* primitive_ptr; 
	};




///////////////////////////////////////////////////////////////////////////////////////////////
//default configurations
	typedef basic_config<> basic_config_t;
	typedef basic_config_t::spectrum_t spectrum_t;
	typedef basic_config_t::transform_t transform_t;

	typedef Scene<scene_config<basic_config_t> >* scene_ptr;
	typedef scene_config<basic_config_t>::light_ptr light_ptr;
	
	typedef material_interface_config<basic_config_t>::float_texture_types float_texture_types;
	typedef material_interface_config<basic_config_t>::spectrum_texture_types spectrum_texture_types;
	typedef material_interface_config<basic_config_t>::texture_spectrum_ref shared_spectrum_texture_t;
	typedef material_interface_config<basic_config_t>::texture_scalar_t_ref shared_float_texture_t;
	typedef material_interface_config<basic_config_t>::default_texture_spectrum_t default_spectrum_texture_t;
	typedef material_interface_config<basic_config_t>::default_texture_scalar_t default_float_texture_t;

	typedef primitive_config<basic_config_t>::material_types material_types;
	typedef scene_config<basic_config_t>::camera_types camera_types;
	typedef scene_config<basic_config_t>::light_types light_types;
	typedef scene_config<basic_config_t>::surface_integrator_types surface_integrator_types;
	typedef scene_config<basic_config_t>::sampler_types sampler_types;
	
	
	typedef camera_config<basic_config_t>::film_types film_types;
	typedef film_config<basic_config_t>::filter_types filter_types;

	typedef geometry_primitive_config<basic_config_t> geometry_primitive_config_t;
	typedef geometry_primitive_config_t::shape_t shape_t;
	typedef geometry_primitive_config_t::shape_types shape_types;
	typedef geometry_primitive_config_t::const_shape_ref_t const_shape_ref_t;
	typedef geometry_primitive_config_t::const_material_ref_t const_material_ref_t;
	typedef MAPrimitive<primitive_interface_config<basic_config_t> > primitive_t;

	typedef MAGeometryPrimitive<geometry_primitive_config<basic_config_t> > geometry_primitive_t;
	typedef boost::shared_ptr< primitive_t > primitive_ref_t;
	typedef geometry_primitive_config_t::shape_ref_t shape_ref_t;
	typedef geometry_primitive_config_t::material_ref_t material_ref_t;

	typedef scene_config<basic_config_t>::surface_integrator_ptr surface_integrator_ptr;
	typedef scene_config<basic_config_t>::primitive_ptr primitive_ptr;
	typedef scene_config<basic_config_t>::camera_ptr	camera_ptr;
	typedef scene_config<basic_config_t>::sampler_ptr sampler_ptr;

	typedef image_film_config<basic_config_t>::filter_ptr filter_ptr;
	typedef camera_config<basic_config_t>::film_ptr film_ptr;

}

#endif
