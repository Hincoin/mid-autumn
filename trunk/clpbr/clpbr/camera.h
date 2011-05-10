#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "config.h"
#include "geometry.h"
#include "ray.h"
#include "random_number_generator.h"

typedef struct  
{
	point3f_t eye;
	vector3f_t center;
	//todo
	vector3f_t dir,x,y;
} camera_t;

class Camera{
public:
	virtual ray_differential_t GenerateRay(const camera_sample_t& cam_samp) = 0;	
};

INLINE void GenerateCameraRay(OCL_CONSTANT_BUFFER camera_t *camera,
							  Seed* s,
							  const int width, const int height, const int x, const int y, ray_t *ray) {
								  const float invWidth = 1.f / width;
								  const float invHeight = 1.f / height;
								  const float r1 = random_float(s) - .5f;
								  const float r2 = random_float(s) - .5f;
								  const float kcx = (x + r1) * invWidth - .5f;
								  const float kcy = (y + r2) * invHeight - .5f;

								  vector3f_t rdir;
								  vinit(rdir,
									  camera->x.x * kcx + camera->y.x * kcy + camera->dir.x,
									  camera->x.y * kcx + camera->y.y * kcy + camera->dir.y,
									  camera->x.z * kcx + camera->y.z * kcy + camera->dir.z);

								  vector3f_t rorig;
								  vsmul(rorig, 0.1f, rdir);
								  vadd(rorig, rorig, camera->eye)

									  vnorm(rdir);
								  rinit(*ray, rorig, rdir);
}

#endif