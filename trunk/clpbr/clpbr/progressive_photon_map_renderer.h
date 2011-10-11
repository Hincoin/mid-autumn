#ifndef _PROGRESSIVE_PHOTON_MAP_RENDERER_H_
#define _PROGRESSIVE_PHOTON_MAP_RENDERER_H_


#include <vector>
#include "renderer.h"
#include "ray_buffer.h"


class Sampler;
class Camera;
class Film;


class PPMRenderer:public Renderer
{
public:
	PPMRenderer(Camera* c,Film* im,Sampler* s)
		:camera_(c),image_(im),sampler_(s)
	{alpha_ = 0.618f;}

	virtual void Render(const Scene* scene);
	virtual ~PPMRenderer();
private:
	Camera* camera_;
	Film* image_;
	Sampler* sampler_;
	//sppm data
	float alpha_;
};

#endif