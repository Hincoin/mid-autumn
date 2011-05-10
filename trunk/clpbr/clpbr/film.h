#ifndef _FILM_H_
#define _FILM_H_

#include "sampler.h"

class Film
{
public:
	void WriteImage();
	void AddSample(const camera_sample_t& cam_samp,const spectrum_t& c);
	unsigned GetWidth()const;
	unsigned GetHeight()const;
protected:
private:
};

#endif