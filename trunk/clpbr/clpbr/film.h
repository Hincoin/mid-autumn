#ifndef _FILM_H_
#define _FILM_H_

#include <math.h>
#include "sampler.h"
#include "spectrum.h"

class Film
{
public:
	virtual void WriteImage(unsigned progressive_iteration)=0;
	virtual void AddSample(const camera_sample_t& cam_samp,const spectrum_t& c)=0;
	virtual unsigned GetWidth()const=0;
	virtual unsigned GetHeight()const=0;
	virtual ~Film(){};
protected:
private:
};

#endif