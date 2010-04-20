#ifndef _MA_INCLUDED_BASIC_CONFIG_HPP_
#define _MA_INCLUDED_BASIC_CONFIG_HPP_



#include "Vector.hpp"
#include "Ray.hpp"

#include "Color.hpp"
#include "DifferentialGeometry.hpp"
namespace ma
{

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
	struct material_interface_config;
	template<typename B,typename S>
	struct texture_interface_config;
	template<typename B>
	struct bxdf_config;
	template<typename B>
	struct filter_config;


}



#endif
