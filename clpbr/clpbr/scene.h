#ifndef _SCENE_H_
#define _SCENE_H_


class Scene
{
public:
	virtual photon_ray_t GeneratePhotonRay()const = 0;
	virtual void PhotonHit(const RayBuffer<photon_ray_t>& photon_rays,std::vector<photon_ray_hit_point_t>* photon_hits)const = 0;
	virtual void RayHit(const RayBuffer<ray_differential_t>& rays,std::vector<ray_hit_point_t>* ray_hits)const = 0;
protected:
private:
};

#endif