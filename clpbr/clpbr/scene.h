#ifndef _SCENE_H_
#define _SCENE_H_

#include <vector>
#include "photon_ray.h"
#include "ray_buffer.h"
#include "ray_hit_point.h"

class Scene
{
public:
	virtual photon_ray_t GeneratePhotonRay()const = 0;
	virtual void PhotonHit( RayBuffer<photon_ray_t>& photon_rays,std::vector<photon_ray_hit_point_t>* photon_hits)const = 0;
	virtual void RayHit(const RayBuffer<ray_differential_t>& rays,std::vector<ray_hit_point_t>* ray_hits)const = 0;
	virtual ~Scene(){}
protected:
private:
};

#endif