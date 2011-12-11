#ifndef _RANDOM_SAMPLER_H_
#define _RANDOM_SAMPLER_H_
#include "sampler.h"

class RandomNumberGeneratorMT19937;
class RandomSampler:public Sampler
{
public:
	RandomSampler(int x_start,int x_end,
		int y_start,int y_end,int xs,int ys);
	~RandomSampler();
	bool GetNextSample(camera_sample_t* cam_samp);
	void ResetSamplePosition() ;
private:
	int x_pos_,y_pos_,x_pixel_samples_,y_pixel_samples_;
	int sample_pos_;
	RandomNumberGeneratorMT19937 *rng;
};


#endif