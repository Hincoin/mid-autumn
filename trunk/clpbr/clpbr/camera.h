#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "config.h"
#include "geometry.h"
#include "matrix4x4.h"
#include "ray.h"
#include "random_number_generator.h"
#include "sampler.h"
#include "film.h"

class Camera{
public:
	Camera(const transform_t& camera_to_world, Film* f):film_(f){transform_assign(camera_to_world_,camera_to_world);}
	void GenerateRay(const camera_sample_t& camera_sample, ray_differential_t *ray,
		float *weight);	
	virtual void GenerateRay(const camera_sample_t& camera_sample, ray_t *ray, float *weight) = 0;
	virtual ~Camera(){}
protected:
	const Film* GetFilm(){return film_;}
	const transform_t& CameraToWorld(){return camera_to_world_;}
private:
	transform_t camera_to_world_;
	Film* film_;
};

struct screen_window_t{
	explicit screen_window_t(float frame_aspect_ratio);
	void set_window(float xmin,float xmax,float ymin,float ymax);
	float x_min,x_max;//[-1,1]
	float y_min,y_max;//[-1,1]
};
class ProjectiveCamera:public Camera
{
public:
	ProjectiveCamera(const transform_t& camera_to_world,const transform_t& proj, const screen_window_t& screen_window,Film *film);
protected:
	const transform_t& CameraToScreen(){return camera_to_screen_;}
	const transform_t& RasterToCamera(){return raster_to_camera_;}
	const transform_t& RasterToScreen(){return raster_to_screen_;}
	const transform_t& ScreenToRaster(){return screen_to_raster_;}
private:
	transform_t camera_to_screen_,raster_to_camera_;
	transform_t raster_to_screen_,screen_to_raster_;
};

//todo: be deleted
typedef struct  
{
	vector3f_t eye;
	vector3f_t dir,x,y;
} camera_t;
INLINE void GenerateCameraRay(OCL_CONSTANT_BUFFER camera_t *camera,
							  Seed* s,
							  const int width, const int height, const int x, const int y, ray_t *ray) {
								  const float invWidth = 1.f / width;
								  const float invHeight = 1.f / height;
								  const float r1 = random_float(s) - .5f;
								  const float r2 = random_float(s) - .5f;
								  const float kcx = (x + r1) * invWidth - .5f;
								  const float kcy = -((y + r2) * invHeight - .5f);

								  vector3f_t rdir;
								  vinit(rdir,
									  camera->x.x * kcx + camera->y.x * kcy + camera->dir.x,
									  camera->x.y * kcx + camera->y.y * kcy + camera->dir.y,
									  camera->x.z * kcx + camera->y.z * kcy + camera->dir.z);

								  vector3f_t rorig;
								  vsmul(rorig, 140, rdir);
								  vadd(rorig, rorig, camera->eye)

									  vnorm(rdir);
								  rinit(*ray, rorig, rdir);
}

#endif