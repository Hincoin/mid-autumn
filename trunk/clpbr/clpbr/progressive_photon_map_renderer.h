#ifndef _PROGRESSIVE_PHOTON_MAP_RENDERER_H_
#define _PROGRESSIVE_PHOTON_MAP_RENDERER_H_


#include <vector>
#include "renderer.h"
#include "ray_buffer.h"
#include "photon_map.h"

class Sampler;
class Camera;
class Film;


class PPMRenderer:public Renderer
{
public:
	PPMRenderer(Camera* c,Film* im,Sampler* s,photon_map_t* photon_map)
		:camera_(c),image_(im),sampler_(s),photon_map_(photon_map)
	{}

	virtual void Render(const cl_scene_info_t scene_info);
	virtual ~PPMRenderer();
private:
	Camera* camera_;
	Film* image_;
	Sampler* sampler_;
	//sppm data
	photon_map_t* photon_map_;
};

#endif