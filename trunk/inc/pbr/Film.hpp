#ifndef _MA_INCLUDED_FILM_HPP_
#define _MA_INCLUDED_FILM_HPP_

namespace ma{
	BEGIN_CRTP_INTERFACE(Film)
	ADD_CRTP_INTERFACE_TYPEDEF(sample_t);
	ADD_CRTP_INTERFACE_TYPEDEF(ray_t);
	ADD_CRTP_INTERFACE_TYPEDEF(spectrum_t);
	ADD_CRTP_INTERFACE_TYPEDEF(scalar_t);
public:
	Film(int xres,int yres):x_resolution(xres),y_resolution(yres){}
	CRTP_METHOD(void,addSample,4,(IN(const sample_t&,s),IN(const ray_t&,r),IN(const spectrum_t&,l),IN(scalar_t,alpha)));
	CRTP_METHOD(void,writeImage,0,());
	CRTP_CONST_VOID_METHOD(getSampleExtent,4,(IN(int&,xs),IN(int&,xe),IN(int&,ys),IN(int&,ye)));
	int xResolution()const{return x_resolution;}
	int yResolution()const{return y_resolution;}
protected:
	const int x_resolution,y_resolution;
	END_CRTP_INTERFACE
}

#endif