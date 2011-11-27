#ifndef _IMAGE_FILM_H_
#define _IMAGE_FILM_H_
#include "film.h"

class MitchellFilter;

class ImageFilm:public Film
{
public:
	ImageFilm(unsigned w,unsigned h);
	void WriteImage(unsigned progressive_iteration);
	void AddSample(const camera_sample_t& cam_samp,const spectrum_t& c);
	unsigned GetWidth()const;
	unsigned GetHeight()const;
	~ImageFilm();
protected:
private:
	unsigned width_,height_;
	struct Pixel{
		Pixel(){
			weight_sum = 0;
		}
		spectrum_t l;
		float weight_sum;
	};
	Pixel *color_back_;
	float *color_buffer_;

	MitchellFilter* filter_;
	float *precomputed_filter_table_;
};


#endif