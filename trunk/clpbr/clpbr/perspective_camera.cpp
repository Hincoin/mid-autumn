#include <cmath>
#include <cstdlib>
#include "perspective_camera.h"

transform_t perspective_transform(float fov_radian, float n, float f)
{
	transform_t perspective;
	transform_init(perspective,
		1,0,0,0,
		0,1,0,0,
		0,0,f/(f-n),-f*n/(f-n),
		0,0,1,0);
	float inv_tan_ang = 1.f/tanf(fov_radian/2);
	transform_t scale;
	transform_identity(scale);
	transform_scale(scale,inv_tan_ang,inv_tan_ang,1);
	transform_t result;
	transform_concat(result, scale, perspective);
	return result;
}
PerspectiveCamera::PerspectiveCamera(const transform_t& camera_to_world,const screen_window_t &screen_window,float field_of_view_radian,Film *f)
:ProjectiveCamera(camera_to_world,perspective_transform(field_of_view_radian, 10.f, 300.f),screen_window, f)
{
	//todo
}

void PerspectiveCamera::GenerateRay(const camera_sample_t &cam_samp, ray_t *ray, float *weight)
{
	point3f_t point_raster={cam_samp.image_x,cam_samp.image_y,0};
	point3f_t point_camera;
	transform_point(point_camera,raster_to_camera_,point_raster);

	point3f_t origin = {0,0,0};
	ray_t r;
	vnorm(point_camera);
	ray_init(r,origin, point_camera);
	r.mint = 0;
	r.maxt = (300.f - 10.f)/r.d.z;

	transform_ray(*ray,camera_to_world_,r);
	*weight = 1.f;


	////////////////////////////////////////////////////
	camera_t camera;

	vinit(camera.eye,50,48,295.6);
	vinit(camera.dir,0,-0.042612,-1);
	vnorm(camera.dir);

	float cx = film_->GetWidth() * 0.5135 / film_->GetHeight();
	vinit(camera.x, cx,0,0);
	vector3f_t cy ;
	vxcross(cy,camera.x,camera.dir);
	vnorm(cy);
	vsmul(camera.y,0.5135f,cy);

	static Seed seed;
	seed.s1 = rand();
	seed.s2 = rand();
	seed.s3 = rand();
	
	GenerateCameraRay(&camera,&seed,film_->GetWidth(),film_->GetHeight(),int(cam_samp.image_x),int (cam_samp.image_y),ray);
}