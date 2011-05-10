#ifndef _SAMPLER_H_
#define _SAMPLER_H_

typedef struct  
{
	float image_x,image_y;
}camera_sample_t;

class Sampler
{
public:
	bool GetNextSample(camera_sample_t* cam_samp);
protected:
private:
};
#endif