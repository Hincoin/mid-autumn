#ifndef _SAMPLER_H_
#define _SAMPLER_H_

typedef struct  
{
	float image_x,image_y;
}camera_sample_t;

class Sampler
{
public:
	Sampler(int x_start,int x_end,
		int y_start,int y_end,int spp):x_pixel_start_(x_start),
		x_pixel_end_(x_end),y_pixel_start_(y_start),y_pixel_end_(y_end),
		samples_per_pixel_(spp){}
	virtual bool GetNextSample(camera_sample_t* cam_samp)=0;
	virtual void ResetSamplePosition() = 0;
	virtual ~Sampler(){}
protected:
	int x_pixel_start_,x_pixel_end_;
	int y_pixel_start_,y_pixel_end_;
	int samples_per_pixel_;
private:
};
#endif