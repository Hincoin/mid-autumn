#ifndef _MA_INCLUDED_FILM_HPP_
#define _MA_INCLUDED_FILM_HPP_

#include "CRTPInterfaceMacro.hpp"

namespace ma{
	BEGIN_CRTP_INTERFACE(Film)
	ADD_CRTP_INTERFACE_TYPEDEF(sample_t);
	ADD_CRTP_INTERFACE_TYPEDEF(ray_t);
	ADD_CRTP_INTERFACE_TYPEDEF(spectrum_t);
	ADD_CRTP_INTERFACE_TYPEDEF(scalar_t);
public:
	Film(int xres,int yres):x_resolution(xres),y_resolution(yres){}
	CRTP_METHOD(void,addSample,4,( I_(const sample_t&,s), I_(const ray_t&,r), I_(const spectrum_t&,l), I_(scalar_t,alpha)));
	CRTP_METHOD(void,writeImage,0,());
	CRTP_CONST_VOID_METHOD(getSampleExtent,4,( I_(int&,xs), I_(int&,xe), I_(int&,ys), I_(int&,ye)));
	int xResolution()const{return x_resolution;}
	int yResolution()const{return y_resolution;}
protected:
	const int x_resolution,y_resolution;
	END_CRTP_INTERFACE
}

#endif