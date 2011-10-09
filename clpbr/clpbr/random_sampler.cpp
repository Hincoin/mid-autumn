#include "random_sampler.h"
#include "random_number_generator.h"

RandomSampler::RandomSampler(int x_start,int x_end,
							 int y_start,int y_end,
							 int xs,int ys)
							 :Sampler(x_start,x_end,y_start,y_end,xs*ys)
{
	x_pos_ = x_start;
	y_pos_ = y_start;
	x_pixel_samples_ = xs;
	y_pixel_samples_ = ys;
	sample_pos_ = 0;
	//random seed
	seed_ = new Seed();
	seed_->s1 = x_start << 16 | x_end;
	seed_->s2 = y_start << 16 | y_end;
	seed_->s3 = x_start << 16 | y_start;
}

void RandomSampler::ResetSamplePosition() 
{
	x_pos_ = x_pixel_start_;
	y_pos_ = y_pixel_start_;
	sample_pos_ = 0;
}
RandomSampler::~RandomSampler()
{
	delete seed_;
}
bool RandomSampler::GetNextSample(camera_sample_t* cam_samp)
{
	//todo
	if(sample_pos_ == samples_per_pixel_)
	{
		if(++x_pos_ == x_pixel_end_)
		{
			x_pos_ = x_pixel_start_;
			y_pos_++;
		}
		if(y_pos_ == y_pixel_end_)
			return false;

		sample_pos_ = 0;
	}

	cam_samp->image_x = x_pos_ + random_float(seed_) ;//- 0.5f;
	cam_samp->image_y = y_pos_ + random_float(seed_) ;//- 0.5f;
	++sample_pos_;
	return true;
}