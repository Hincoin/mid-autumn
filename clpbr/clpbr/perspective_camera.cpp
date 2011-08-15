#include <cmath>
#include "perspective_camera.h"


PerspectiveCamera::PerspectiveCamera(point3f_t eye, point3f_t center)
:eye_(eye),center_(center)
{
	//todo
}

ray_differential_t PerspectiveCamera::GenerateRay(const camera_sample_t &cam_samp)
{
	//todo

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
	return ray_differential_t();
}