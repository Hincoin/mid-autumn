#ifndef _MA_INCLUDED_PERSPECTIVECAMERA_HPP_
#define _MA_INCLUDED_PERSPECTIVECAMERA_HPP_

#include "Camera.hpp"

namespace ma{
	//
	template<typename Conf>
	class PerspectiveCamera:public ProjectiveCamera<PerspectiveCamera<Conf>,typename Conf::interface_config>
	{
		friend class Camera<PerspectiveCamera<Conf>,typename Conf::interface_config>;
		typedef ProjectiveCamera<PerspectiveCamera<Conf>,typename Conf::interface_config> parent_type;
	public:
		ADD_SAME_TYPEDEF(Conf,sample_t);
		ADD_SAME_TYPEDEF(Conf,transform_t);
		ADD_SAME_TYPEDEF(Conf,film_ptr);
		ADD_SAME_TYPEDEF(Conf,scalar_t);
		ADD_SAME_TYPEDEF(Conf,ray_t);
		ADD_SAME_TYPEDEF(Conf,point_t)
	public:
		// PerspectiveCamera Public Methods
		PerspectiveCamera(const transform_t &world2cam,
			const scalar_t Screen[4], scalar_t hither, scalar_t yon,
			scalar_t sopen, scalar_t sclose,
			scalar_t lensr, scalar_t focald, scalar_t fov,
			film_ptr film):
			parent_type(world2cam,
			perspective(fov, hither, yon),
			Screen, hither, yon, sopen, sclose,
			lensr, focald, film) {}
	private:
		scalar_t generateRayImpl(const sample_t &sample, ray_t &) const;
	};

	template<typename Conf>
	typename Conf::scalar_t
		PerspectiveCamera<Conf>::generateRayImpl(const sample_t &sample, ray_t &ray)const
	{
		// Generate raster and camera samples
		point_t Pras(sample.image_x, sample.image_y, 0);
		//point_t Pras(24.0711,2.3352,0);
		point_t Pcamera;
		Pcamera.p = parent_type::RasterToCamera * Pras.p;

		ray.o = Pcamera;
		ray.dir=(typename parent_type::vector_t(Pcamera.x(), Pcamera.y(), Pcamera.z()));
		// Set ray time value
		//ray.time =lerp(shutter_open,shutter_close,sample.time);
		// Modify ray for depth of field
		if (parent_type::LensRadius > 0.) {
			// Sample point on lens
			float lensU, lensV;
			ConcentricSampleDisk(sample.lens_u, sample.lens_v,
				&lensU, &lensV);
			lensU *= parent_type::LensRadius;
			lensV *= parent_type::LensRadius;
			// Compute point on plane of focus
			float ft = (parent_type::FocalDistance - parent_type::clip_hither) / ray.dir.z();
			point_t Pfocus = ray.o + (ray.dir) * (ft);
			// Update ray for effect of lens
			ray.o.x() += lensU;
			ray.o.y() += lensV;
			ray.dir = Pfocus - ray.o;
		}
		ray.dir.normalize();
		ray.mint = 0.;
		ray.maxt = (parent_type::clip_yon - parent_type::clip_hither) / ray.dir.z();

		ray=(parent_type::camera_to_world * ray) ;

		return scalar_t(1);
	}
}

namespace ma
{
	MAKE_TYPE_STR_MAP(1,PerspectiveCamera,perspective)
namespace details
{
template<typename Conf>
	struct camera_creator<PerspectiveCamera<Conf> >
	{
		typedef PerspectiveCamera<Conf> camera_t;
		camera_t* operator()(const ParamSet& param,const typename camera_t::transform_t& world_to_camera,
				const typename camera_t::film_ptr film)const
		{
	 //////////////////////////////////////////////////////////////////////////
	 // Extract common camera parameters
	 float hither = 1e-3f;
	 float yon =  1e30f;
	 float shutteropen =  0.f;
	 float shutterclose = 1.f;
	 float lensradius =  0.f;
	 float focaldistance =  1e30f;
	 float frame = 1.33f;
	 float screen[4];
	 if (frame > 1.f) {
		 screen[0] = -frame;
		 screen[1] =  frame;
		 screen[2] = -1.f;
		 screen[3] =  1.f;
	 }
	 else {
		 screen[0] = -1.f;
		 screen[1] =  1.f;
		 screen[2] = -1.f / frame;
		 screen[3] =  1.f / frame;
	 }
	 float fov =  90;
	 return new camera_t(world_to_camera,screen, hither, yon,
		 shutteropen, shutterclose, lensradius, focaldistance,
		 fov, film);
		
		}	
	};
}
}
#endif
