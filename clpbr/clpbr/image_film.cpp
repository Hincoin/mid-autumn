#include "image_film.h"





ImageFilm::ImageFilm(unsigned w,unsigned h):
width_(w),height_(h)
{}
void ImageFilm::WriteImage()
{}
void ImageFilm::AddSample(const camera_sample_t& cam_samp,const spectrum_t& c)
{}
unsigned ImageFilm::GetWidth()const
{
	return width_;
}
unsigned ImageFilm::GetHeight()const
{
	return height_;
}



