#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mitchell_filter.h"
#include "image_film.h"

#include <malloc.h>


int toInt(double x)
{
	return int(pow(1-exp(-x),1/2.2)*255+.5);
	//return int(255*x);
} //tone mapping

static const int FILTER_TABLE_SIZE = 16;

ImageFilm::ImageFilm(unsigned w,unsigned h,unsigned int wr):
width_(w),height_(h),write_frequency_(wr),current_iteration_(0),current_sample_count_(0)
{
	filter_ = new MitchellFilter(1.f/3.f,1.f/3.f,2.f,2.f);
	color_back_ = new Pixel[w*h];
	color_buffer_ = (float*)calloc(w*h*3,sizeof(float));

	precomputed_filter_table_ = new float[FILTER_TABLE_SIZE * FILTER_TABLE_SIZE];
	float *pft = precomputed_filter_table_;
	for( int y = 0; y < FILTER_TABLE_SIZE; ++y)
	{
		float fy = ((float)y + 0.5f) * filter_->YWidth() / FILTER_TABLE_SIZE;
		for( int x = 0; x < FILTER_TABLE_SIZE; ++x)
		{
			float fx = ((float)x + 0.5f) * filter_->XWidth() / FILTER_TABLE_SIZE;
			*pft++ = filter_->Evaluate(fx,fy);
		}
	}
}
ImageFilm::~ImageFilm()
{
	delete []filter_;
	delete []color_back_;
	free(color_buffer_);
}
void ImageFilm::WriteImage(unsigned progressive_iteration)
{
	current_iteration_ = progressive_iteration;
	char file_name[256] = {0};
	sprintf(file_name,"image_%d.ppm",progressive_iteration);
	FILE* f = fopen(file_name,"w"); 
	fprintf(f,"P3\n%d %d\n%d\n",width_,height_,255);
	for(unsigned int i = 0; i< width_ * height_; i++) 
	{
		unsigned int idx = 3*i;
		if(color_back_[i].weight_sum != 0)
		{
			float invert_weight_sum = 1.f/color_back_[i].weight_sum;
			float r=(color_back_[i].l.x * invert_weight_sum +color_buffer_[idx]*progressive_iteration)/float(progressive_iteration+1);
			float g=(color_back_[i].l.y * invert_weight_sum +color_buffer_[idx+1]*progressive_iteration)/float(progressive_iteration+1);
			float b=(color_back_[i].l.z * invert_weight_sum +color_buffer_[idx+2]*progressive_iteration)/float(progressive_iteration+1);
			color_buffer_[idx]   = r;
			color_buffer_[idx+1] = g;
			color_buffer_[idx+2] = b;
		}
		fprintf(f,"%d %d %d ", toInt(color_buffer_[idx]),toInt(color_buffer_[idx+1]),toInt(color_buffer_[idx+2]));
	}
	::fflush(f);
	::fclose(f);
	::memset(color_back_,0,width_*height_* sizeof(color_back_[0]));
}
void ImageFilm::AddSample(const camera_sample_t& camera_sample,const spectrum_t& c)
{

	float d_image_x = camera_sample.image_x - 0.5f;
	float d_image_y = camera_sample.image_y - 0.5f;
	int x0 = (int)ceil(d_image_x - filter_->XWidth());
	int x1 = (int)floor(d_image_x + filter_->XWidth());
	int y0 = (int)ceil(d_image_y - filter_->YWidth());
	int y1 = (int)floor(d_image_y + filter_->YWidth());
	
	int x_start = 0;
	int y_start = 0;
	x0 = max(x0,x_start);
	x1 = min(x1,x_start + (int)width_ - 1);
	y0 = max(y0,y_start);
	y1 = min(y1,y_start + (int)width_ - 1);
	if( x1 < x0 || y1 < y0 ) return;

	int *ifx = (int*)alloca((x1 - x0 + 1) * sizeof(int));
	for(int x = x0; x <= x1; ++x)
	{
		float fx = fabsf((x - d_image_x) * filter_->InverseXWidth() * FILTER_TABLE_SIZE);
		ifx[x - x0] = min((int)floor(fx), FILTER_TABLE_SIZE - 1);
	}
	int *ify = (int*)alloca((y1 - y0 + 1) * sizeof(int));
	for(int y = y0; y <= y1; ++y)
	{
		float fy = fabsf((y - d_image_y) * filter_->InverseYWidth() * FILTER_TABLE_SIZE);
		ify[y - y0] = min((int)floor(fy), FILTER_TABLE_SIZE - 1);
	}
	spectrum_t current_color;
	for(int y = y0; y <= y1; ++y)
		for( int x = x0; x <= x1; ++x)
		{
			int offset = ify[y-y0] * FILTER_TABLE_SIZE + ifx[x-x0];
			float filter_weight = precomputed_filter_table_[offset];
			int idx = y * width_ + x;
			vsmul(current_color,filter_weight, c);
			vadd(color_back_[idx].l , color_back_[idx].l, current_color);
			color_back_[idx].weight_sum += filter_weight;
			//color_back_[idx].l = c;//current_color;
			//color_back_[idx].weight_sum = 1.f;
		}
	if(write_frequency_  == ++current_sample_count_)
	{
		current_sample_count_ = 0;
		WriteImage(current_iteration_);
	}
}
unsigned ImageFilm::GetWidth()const
{
	return width_;
}
unsigned ImageFilm::GetHeight()const
{
	return height_;
}



