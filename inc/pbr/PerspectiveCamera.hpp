#ifndef _MA_INCLUDED_PERSPECTIVECAMERA_HPP_
#define _MA_INCLUDED_PERSPECTIVECAMERA_HPP_

#include "Camera.hpp"

namespace ma{
	//
	template<typename Conf>
	class PerspectiveCamera:public ProjectiveCamera<PerspectiveCamera<Conf>,Conf>
	{
		friend class Camera<PerspectiveCamera<Conf>,Conf>;
		typedef ProjectiveCamera<PerspectiveCamera<Conf>,Conf> parent_type;
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
		Pcamera.p = RasterToCamera * Pras.p;

		ray.o = Pcamera;
		ray.dir = vector_t(Pcamera.x(), Pcamera.y(), Pcamera.z());
		// Set ray time value
		//ray.time =lerp(shutter_open,shutter_close,sample.time);
		// Modify ray for depth of field
		if (LensRadius > 0.) {
			// Sample point on lens
			float lensU, lensV;
			ConcentricSampleDisk(sample.lens_u, sample.lens_v,
				&lensU, &lensV);
			lensU *= LensRadius;
			lensV *= LensRadius;
			// Compute point on plane of focus
			float ft = (FocalDistance - clip_hither) / ray.dir.z();
			point_t Pfocus = ray.o + (ray.dir) * (ft);
			// Update ray for effect of lens
			ray.o.x() += lensU;
			ray.o.y() += lensV;
			ray.dir = Pfocus - ray.o;
		}
		ray.dir.normalize();
		ray.mint = 0.;
		ray.maxt = (clip_yon - clip_hither) / ray.dir.z();

		ray = camera_to_world * ray;

		return scalar_t(1);
	}
}
#endif