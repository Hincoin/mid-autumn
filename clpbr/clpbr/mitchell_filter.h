#ifndef _MITCHELL_FILTER_H_
#define _MITCHELL_FILTER_H_

#include <math.h>
class MitchellFilter
{
private:
	const float x_width_,y_width_;
	const float inverse_x_width_,inverse_y_width_;
public:
	MitchellFilter(float b,float c,float xw,float yw)
		:x_width_(xw),y_width_(yw),b_(b),c_(c),inverse_x_width_(1.f/xw),inverse_y_width_(1.f/yw){
	}
	float XWidth()const{return x_width_;}
	float YWidth()const{return y_width_;}
	float InverseXWidth()const{return inverse_x_width_;}
	float InverseYWidth()const{return inverse_y_width_;}

	float Evaluate(float x, float y)const
	{
		return Mitchell1D(x * inverse_x_width_) * Mitchell1D(y * inverse_y_width_);
	}
private:
	float Mitchell1D(float x)const {
		x = fabsf(2.f * x);
		static const float one_over_six = 1.f/6.f;
		if (x > 1.f)
			return ((-b_ - 6 * c_) * x * x * x + 
					(6*b_ + 30 * c_) * x * x + 
					(-12 * b_ - 48 * c_) * x + 
					(8 * b_ + 24 * c_)) * one_over_six;
		else
			return ((12 - 9 * b_ - 6 * c_) * x * x * x +
					(-18 + 12 * b_ + 6 * c_) * x * x + 
					//() * x +
					(6 - 2 * b_)) * one_over_six;
	}
private:
	const float b_,c_;
};


#endif