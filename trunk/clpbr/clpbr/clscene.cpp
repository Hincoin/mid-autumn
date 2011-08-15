#include <cmath>
#include "clscene.h"



CLScene::CLScene(const cl_scene_info_t &scene_info)
{
	//todo
}
photon_ray_t CLScene::GeneratePhotonRay() const
{
	//todo
	return photon_ray_t();
}
void CLScene::PhotonHit(const RayBuffer<photon_ray_t> &photon_rays, std::vector<photon_ray_hit_point_t> *photon_hits)const
{
	//todo
}
void CLScene::RayHit(const RayBuffer<ray_differential_t> &rays, std::vector<ray_hit_point_t> *ray_hits)const
{
	//todo
}
