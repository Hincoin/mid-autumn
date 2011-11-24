#ifndef _PROGRESSIVE_PHOTON_MAP_RENDERER_H_
#define _PROGRESSIVE_PHOTON_MAP_RENDERER_H_


#include <vector>
#include "renderer.h"
#include "ray_buffer.h"
#include "photon_map.h"

class Sampler;
class Camera;
class Film;

class OpenCLDevice;
struct scene_info_memory_t;

class PPMRenderer:public Renderer
{
public:
	PPMRenderer(Camera* c,Film* im,Sampler* s,photon_map_t* photon_map);

	virtual void Render(const scene_info_memory_t& scene_info);
	virtual ~PPMRenderer();
private:
	void InitializeDeviceData(const scene_info_memory_t& scene_info);
private:
	Camera* camera_;
	Film* image_;
	Sampler* sampler_;
	//sppm data
	photon_map_t* photon_map_;

	//
	OpenCLDevice* device_;
};

#endif