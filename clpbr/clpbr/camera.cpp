#include <math.h>
#include "camera.h"




void Camera::GenerateRay(const camera_sample_t &camera_sample, ray_differential_t *ray, float *weight)
{
	GenerateRay(camera_sample, static_cast<ray_t*>(ray), weight);
	ray->has_differential = 0;
	//todo
	//shift x and y direction
}


ProjectiveCamera::ProjectiveCamera(const transform_t& camera_to_world,const transform_t &proj, const screen_window_t &screen_window, Film *f)
:Camera(camera_to_world,f)
{
	transform_assign(camera_to_screen_, proj);

	transform_t scale0,scale1,translate;
	transform_t transform_tmp;

	transform_identity(scale0);
	transform_identity(scale1);
	transform_identity(translate);

	transform_identity(screen_to_raster_);
	transform_scale(scale0,float(film_->GetWidth()),float(film_->GetHeight()),1.f);
	transform_scale(scale1,1.f/(screen_window.x_max-screen_window.x_min),1.f/(screen_window.y_min - screen_window.y_max),1.f);
	transform_translate(translate, -screen_window.x_min, -screen_window.y_max,0.f);

	transform_concat(transform_tmp, scale0, scale1);
	transform_concat(screen_to_raster_,transform_tmp, translate);
	transform_inverse(raster_to_screen_,screen_to_raster_);

	transform_inverse(transform_tmp, camera_to_screen_);
	transform_concat(raster_to_camera_,transform_tmp,raster_to_screen_);

}