#ifndef _SPPM_RENDERER_H_
#define _SPPM_RENDERER_H_

#include <vector>
#include "renderer.h"
#include "ray_buffer.h"
#include "sppm_hit_point.h"


class Sampler;
class Camera;
class Film;


class SPPMHashGrid;

class SPPMRenderer:public Renderer
{
public:
	SPPMRenderer(Camera* c,Film* im,Sampler* s)
		:camera_(c),image_(im),sampler_(s){}

	virtual void Render(const Scene* scene);
	virtual ~SPPMRenderer();
protected:
	void PhotonTrace(const Scene* scene,
		const SPPMHashGrid& hash_grid,const std::vector<ray_hit_point_t>& ray_hits,
		std::vector<accum_hit_point_t>& accum_hits);
private:
	Camera* camera_;
	Film* image_;
	Sampler* sampler_;
	//sppm data
	float alpha_;
	unsigned num_photons_per_pass_;
	mutable float max_photon_r2_;
	unsigned num_total_photons_;
};

#endif
