#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "image_film.h"


int toInt(double x){return int(pow(1-exp(-x),1/2.2)*255+.5);} //tone mapping

ImageFilm::ImageFilm(unsigned w,unsigned h):
width_(w),height_(h)
{
	color_array_ = (unsigned char*)calloc(w*h*3,sizeof(char));
}
ImageFilm::~ImageFilm()
{
	free(color_array_);
}
void ImageFilm::WriteImage()
{
	  FILE* f = fopen("image.ppm","w"); 
	  fprintf(f,"P3\n%d %d\n%d\n",width_,height_,255);
	  for(int i = 0; i< width_ * height_; i++) 
	  {
		  int idx = 3*i;
          fprintf(f,"%d %d %d ", color_array_[idx],color_array_[idx+1],color_array_[idx+2]);
	  }
	  ::fflush(f);
	  ::fclose(f);
	  ::memset(color_array_,0,width_*height_*3*sizeof(char));
}
void ImageFilm::AddSample(const camera_sample_t& camera_sample,const spectrum_t& c)
{
	int widx = (int)camera_sample.image_x;
	int hidx = (int)camera_sample.image_y;
	unsigned idx = hidx * width_ + widx;
	color_array_[3*idx] = toInt(c.x);
	color_array_[3*idx + 1] = toInt(c.y);
	color_array_[3*idx+2] = toInt(c.z);
	return;
	//todo
	if(!color_is_black(c))
		printf("add sample at (%.3f,%.3f) value:(%d,%d,%d)\n",
			camera_sample.image_x,camera_sample.image_y,color_array_[3*idx],color_array_[3*idx+1],color_array_[3*idx+2]);
}
unsigned ImageFilm::GetWidth()const
{
	return width_;
}
unsigned ImageFilm::GetHeight()const
{
	return height_;
}



