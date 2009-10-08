#ifndef _MA_INCLUDED_CAMERA_HPP_
#define _MA_INCLUDED_CAMERA_HPP_

#include "CRTPInterfaceMacro.hpp"
#include "Move.hpp"
#include "PtrTraits.hpp"

namespace ma{
	BEGIN_CRTP_INTERFACE(Camera)
	ADD_CRTP_INTERFACE_TYPEDEF(sample_t);
	ADD_CRTP_INTERFACE_TYPEDEF(transform_t);
	ADD_CRTP_INTERFACE_TYPEDEF(film_ptr);
	ADD_CRTP_INTERFACE_TYPEDEF(scalar_t);
	ADD_CRTP_INTERFACE_TYPEDEF(ray_t);
	ADD_CRTP_INTERFACE_TYPEDEF(spectrum_t)

	Camera(const transform_t &world2cam,scalar_t hither,scalar_t yon,
		scalar_t sopen,scalar_t sclose,film_ptr film):film_(film),world_to_camera(world2cam),
		camera_to_world(ma::force_move(world2cam.inverse())),
		clip_hither(hither),
		clip_yon(yon),
		shutter_open(sopen),
		shutter_close(sclose)

	{}

	~Camera(){delete_ptr(film_);}
	CRTP_CONST_METHOD(scalar_t,generateRay,2,
		( I_(const sample_t&,sample), I_(ray_t&,r)));

	void addSample(const sample_t& s,const ray_t& r,const spectrum_t& l,scalar_t alpha)
	{return film_->addSample(s,r,l,alpha);}
	void writeImage(){return film_->writeImage();}
protected:
	film_ptr film_;
	transform_t world_to_camera,camera_to_world;
	scalar_t clip_hither,clip_yon;
	scalar_t shutter_open,shutter_close;
	END_CRTP_INTERFACE

	template<typename Derived,typename Conf>
	class ProjectiveCamera:public Camera<Derived,Conf>
	{
	public:
		ADD_SAME_TYPEDEF(Conf,transform_t)
		ADD_SAME_TYPEDEF(Conf,vector_t)
		ADD_SAME_TYPEDEF(Conf,scalar_t)
		ADD_SAME_TYPEDEF(Conf,film_ptr)
		typedef Camera<Derived,Conf> parent_type;
	public:
		// ProjectiveCamera Public Methods
		ProjectiveCamera(const transform_t &world2cam,
			const transform_t &proj, const scalar_t Screen[4],
			scalar_t hither, scalar_t yon,
			scalar_t sopen, scalar_t sclose,
			scalar_t lensr, scalar_t focald, film_ptr film)
			: parent_type(world2cam, hither, yon, sopen, sclose, film) {
				// Initialize depth of field parameters
				LensRadius = lensr;
				FocalDistance = focald;
				// Compute projective camera transformations
				CameraToScreen = proj;
				WorldToScreen = CameraToScreen * world2cam;
				// Compute projective camera screen transformations
				ScreenToRaster.identity();

				ScreenToRaster.scale(vector_t((scalar_t)film->xResolution(),scalar_t(film->yResolution()), 1.f)).scale
					(vector_t(1.f / (Screen[1] - Screen[0]),1.f / (Screen[2] - Screen[3]), 1.f)).translate
					(vector_t(-Screen[0], -Screen[3], 0.f));

				RasterToScreen = ScreenToRaster.inverse();
				transform_t tmp = CameraToScreen.inverse();
				RasterToCamera =
					CameraToScreen.inverse() * RasterToScreen;
		}
	protected:
		// ProjectiveCamera Protected Data
		transform_t CameraToScreen, WorldToScreen, RasterToCamera;
		transform_t ScreenToRaster, RasterToScreen;
		scalar_t LensRadius, FocalDistance;
	};



}
#endif
