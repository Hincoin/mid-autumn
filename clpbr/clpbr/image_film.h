#ifndef _IMAGE_FILM_H_
#define _IMAGE_FILM_H_
#include "film.h"


class ImageFilm:public Film
{
public:
	ImageFilm(unsigned w,unsigned h);
	void WriteImage();
	void AddSample(const camera_sample_t& cam_samp,const spectrum_t& c);
	unsigned GetWidth()const;
	unsigned GetHeight()const;
protected:
private:
	unsigned width_,height_;
};


#endif