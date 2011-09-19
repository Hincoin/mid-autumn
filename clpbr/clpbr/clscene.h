#ifndef _CLSCENE_H_
#define _CLSCENE_H_

#include "scene.h"
#include "cl_scene.h"
#include "ray_hit_point.h"

class CLScene:public Scene{
public:
	CLScene(const cl_scene_info_t& scene_info);
	virtual photon_ray_t GeneratePhotonRay()const;
	virtual void PhotonHit( RayBuffer<photon_ray_t>& photon_rays,std::vector<photon_ray_hit_point_t>* photon_hits)const;
	virtual void RayHit(const RayBuffer<ray_differential_t>& rays,std::vector<ray_hit_point_t>* ray_hits)const;
	~CLScene();
private:
	cl_scene_info_t scene_info_;

	float *lights_power_;
	float *light_cdf_;
	float total_power_;
};



#endif