#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "image_film.h"


int toInt(double x)
{
	return int(pow(1-exp(-x),1/2.2)*255+.5);
	//return int(255*x);
} //tone mapping

ImageFilm::ImageFilm(unsigned w,unsigned h):
width_(w),height_(h)
{
	color_array_ = (float*)calloc(w*h*3,sizeof(float));
	color_buffer_ = (float*)calloc(w*h*3,sizeof(float));
}
ImageFilm::~ImageFilm()
{
	free(color_array_);
	free(color_buffer_);
}
void ImageFilm::WriteImage(unsigned progressive_iteration)
{
	int debug_non_black_count = 0;
	  FILE* f = fopen("image.ppm","w"); 
	  fprintf(f,"P3\n%d %d\n%d\n",width_,height_,255);
	  for(int i = 0; i< width_ * height_; i++) 
	  {
		  int idx = 3*i;
		  if(color_array_[idx] > 0 && color_array_[idx+1] > 0 && color_array_[idx+2] > 0)
			  debug_non_black_count ++;
		  float r=(color_array_[idx]+color_buffer_[idx]*progressive_iteration)/float(progressive_iteration+1);
		  float g=(color_array_[idx+1]+color_buffer_[idx+1]*progressive_iteration)/float(progressive_iteration+1);
		  float b=(color_array_[idx+2]+color_buffer_[idx+2]*progressive_iteration)/float(progressive_iteration+1);
		  color_buffer_[idx]   = r;
		  color_buffer_[idx+1] = g;
		  color_buffer_[idx+2] = b;

          fprintf(f,"%d %d %d ", toInt(r),toInt(g),toInt(b));
	  }
	  ::fflush(f);
	  ::fclose(f);
	  ::memset(color_array_,0,width_*height_*3*sizeof(float));
	  printf("non_black_count:%d\n",debug_non_black_count);
}
void ImageFilm::AddSample(const camera_sample_t& camera_sample,const spectrum_t& c)
{
	int widx = (int)camera_sample.image_x;
	int hidx = (int)camera_sample.image_y;
	unsigned idx = hidx * width_ + widx;
	color_array_[3*idx] = (c.x);
	color_array_[3*idx + 1] = (c.y);
	color_array_[3*idx+2] = (c.z);
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



