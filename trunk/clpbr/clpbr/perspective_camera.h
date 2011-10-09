#include "camera.h"

#include "radian_degree.h"

class PerspectiveCamera:public ProjectiveCamera
{
public:
	PerspectiveCamera(const transform_t& camera_to_world,const screen_window_t &screen_window,radian_t field_of_view,Film *f);
	virtual void GenerateRay(const camera_sample_t& cam_samp, ray_t *ray, float *weight);
protected:
private:
	
};