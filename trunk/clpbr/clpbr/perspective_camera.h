#include "camera.h"


class PerspectiveCamera:public Camera
{
public:
	PerspectiveCamera(point3f_t eye,point3f_t center);
	ray_differential_t GenerateRay(const camera_sample_t& cam_samp);
protected:
private:
	point3f_t center_,eye_;
};