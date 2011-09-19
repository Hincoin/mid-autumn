#include "image_film.h"

#include <stdio.h>




ImageFilm::ImageFilm(unsigned w,unsigned h):
width_(w),height_(h)
{}
void ImageFilm::WriteImage()
{}
void ImageFilm::AddSample(const camera_sample_t& camera_sample,const spectrum_t& c)
{
	//todo
	printf("add sample at (%.3f,%.3f) value:(%.3f,%.3f,%.3f)\n",
		camera_sample.image_x,camera_sample.image_y,c.x,c.y,c.z);
}
unsigned ImageFilm::GetWidth()const
{
	return width_;
}
unsigned ImageFilm::GetHeight()const
{
	return height_;
}



