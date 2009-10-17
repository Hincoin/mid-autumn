#ifndef _MA_INCLUDED_DEFAULT_CONFIGURATIONS_HPP_
#define _MA_INCLUDED_DEFAULT_CONFIGURATIONS_HPP_

#include "Vector.hpp"
#include "Ray.hpp"

namespace ma{



}

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
#include "WhittedIntegrator.hpp"
#include "TriangleMesh.hpp"
#include "Mitchell.hpp"
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

		typedef Spectrum< scalar_t> spectrum_t;
		//////////////////////////////////////////////////////////////////////////
		//temp typedef
		typedef ray_t ray_differental_t;

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
	struct film_config:B{

		typedef CameraSample<sample_config<B> > sample_t;

		typedef MitchellFilter<filter_config<B> >* filter_ptr;
	};
	template<typename B>
	struct camera_config:B{
		typedef CameraSample<sample_config<B> > sample_t;
		typedef ImageFilm<film_config<B> >* film_ptr;
		//ADD_CRTP_INTERFACE_TYPEDEF(film_ptr);
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
		//typedef Sample<sample_config<B> > sample_t;
		typedef typename scene_config<B>::sample_t sample_t;
	};
	template<typename B>
	struct sample_config:B{
		//typedef Scene<scene_config<B> >* scene_ptr;
		//typedef WhittedIntegrator<surface_integrator_config<B> >* surface_integrator_ptr;
		//ADD_SAME_TYPEDEF(Conf,surface_integrator_ptr);
		//ADD_SAME_TYPEDEF(Conf,volume_integrator_ptr);
		//typedef void* volume_integrator_ptr;
	};
	template<typename B,typename S>
	struct texture_config {
		typedef DifferentialGeometry<typename B::scalar_t,B::dimension> differential_geometry_t;
		typedef S scalar_t;
	};
	template<typename B>
	struct material_config:public B{
		typedef boost::shared_ptr<BSDF<bsdf_config<B> > > bsdf_ptr;

	typedef ConstantTexture<texture_config<B,typename B::spectrum_t> > texture_spectrum_t;
	typedef ConstantTexture<texture_config<B,typename B::scalar_t> > texture_scalar_t;
	typedef boost::shared_ptr<texture_spectrum_t>  texture_spectrum_ref;
	typedef boost::shared_ptr<texture_scalar_t> texture_scalar_t_ref;


	typedef DifferentialGeometry<typename B::scalar_t,B::dimension> differential_geometry_t;

	//the following is matte material specific config
	typedef BSDF<bsdf_config<B> > bsdf_t;
	typedef Lambertian<bxdf_config<B> > lambertian_t;
	typedef OrenNayar<bxdf_config<B> > oren_nayar_t;
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


		typedef Matte<material_config<B> > material_t;

		//typedef typename Conf::ScalarType ScalarType;
		//static const int Dimension = Conf::Dimension;
		//typedef typename vector_type<ScalarType,Dimension>::type VectorType;
		//typedef SpaceSegment<VectorType> BBox;
		//typedef  Point<VectorType> point_type;
		//typedef Ray<VectorType> ray_type;
		//typedef MAIntersection<PrimitiveConfigure<ScalarType,Dimension> > intersection_type;
		//typedef MAPrimitive<Conf>  interface_type;
		//typedef typename shared_pointer<interface_type>::type shared_primitive;
		//typedef std::vector<shared_primitive> shared_primitive_array;
		//typedef Primitive<Derived,Conf> class_type;
	};
	template<typename B>
	struct geometry_primitive_config:primitive_config<B>{
	    typedef primitive_config<B> parent_type;
		typedef MATriangleMesh<> shape_t;
		typedef MAGeometryPrimitive<refined_primitive_config<B> > refined_primitive_type;
		typedef MATriangle<> refined_shape_t;
		typedef boost::shared_ptr<refined_shape_t> refined_shape_ref_t;

		typedef const boost::shared_ptr<shape_t> const_shape_ref_t;
		typedef const boost::shared_ptr<typename parent_type::material_t> const_material_ref_t;
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
	struct volume_region_config:B{};

	template<typename B>
	struct bxdf_config:public B{

	};
	template<typename B>
	struct bsdf_config:public B{
		typedef DifferentialGeometry<typename B::scalar_t,B::dimension> differential_geometry_t;
		//typedef Lambertian<bxdf_config<B> >* BxDF_ptr;
		typedef ptr_var<Lambertian<bxdf_config<B> >,OrenNayar<bxdf_config<B> > > BxDF_ptr;
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
	typedef PointLight<light_config<B> > light_t;
	typedef light_t* light_ptr; //ptr var
	typedef PerspectiveCamera<camera_config<B> >* camera_ptr;
	typedef WhittedIntegrator<surface_integrator_config<B> >* surface_integrator_ptr;
	typedef LDSampler<sampler_config<B> >* sampler_ptr;
	typedef Sample<sample_config<B> > sample_t;
	typedef sample_t* sample_ptr;
	typedef MAPrimitive<primitive_interface_config<B> >* primitive_ptr;
	typedef Intersection<intersection_config<B> > intersection_t;
	//typedef typename intersection_config<B>::ray_differential_t ray_differential_t;
	typedef RayDifferential<typename B::vector_t> ray_differential_t;
	typedef void* volume_integrator_ptr;
	typedef void* volume_region_ptr;

		//ADD_SAME_TYPEDEF(Conf,volume_integrator_ptr)
		//ADD_SAME_TYPEDEF(Conf,volume_region_ptr)
		//ADD_SAME_TYPEDEF(Conf,intersection_t)
	};


//default configurations
	typedef basic_config<> basic_config_t;
	typedef Scene<scene_config<basic_config_t> >* scene_ptr;
	typedef scene_config<basic_config_t>::light_t light_t;
	typedef scene_config<basic_config_t>::light_ptr light_ptr;


	typedef geometry_primitive_config<basic_config_t> geometry_primitive_config_t;
	typedef geometry_primitive_config_t::shape_t shape_t;
	typedef geometry_primitive_config_t::material_t material_t;
	typedef geometry_primitive_config_t::const_shape_ref_t const_shape_ref_t;
	typedef geometry_primitive_config_t::const_material_ref_t const_material_ref_t;
	typedef MAPrimitive<primitive_interface_config<basic_config_t> > primitive_t;

	typedef MAGeometryPrimitive<geometry_primitive_config<basic_config_t> > geometry_primitive_t;
	typedef boost::shared_ptr< primitive_t > primitive_ref_t;
}
#endif
