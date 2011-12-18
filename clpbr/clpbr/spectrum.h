#ifndef _SPECTRUM_H_
#define _SPECTRUM_H_
/*
typedef 
	union{
		struct {float x,y,z;};
		struct {float r,g,b;};
	}spectrum_t;
	*/
//because of compiler bug, cannot use union here
#ifndef CL_KERNEL
struct spectrum_t
{
	spectrum_t(float xx=0,float yy=0, float zz=0,float aa=0):x(xx),y(yy),z(zz),w(aa){}
	float x,y,z,w;
};
#else 
#define spectrum_t float4

#endif

#define color_is_black(_x) ((_x).x == 0.f && (_x).y == 0.f && (_x).z == 0.f)

#define color_clamp(_x,_low,_high) {clamp((_x).x,(_low),(_high));clamp((_x).y,(_low),(_high));clamp((_x).z,(_low),(_high));};

#include "geometry.h"
INLINE int convert_to_rgb(const spectrum_t* spectrum)
{
	//float xyz[3];
	//float c[3]={spectrum->x,spectrum->y,spectrum->z};
	//float XWeight[] = {
	//	0.412453f, 0.357580f, 0.180423f
	//};
	//float YWeight[] = {
	//	0.212671f, 0.715160f, 0.072169f
	//};
	//float ZWeight[] = {
	//	0.019334f, 0.119193f, 0.950227f
	//};
	//const int COLOR_SAMPLES=3;
	//xyz[0] = xyz[1] = xyz[2] = 0.;
	//	for (int i = 0; i < COLOR_SAMPLES; ++i) {
	//		xyz[0] += XWeight[i] * c[i];
	//		xyz[1] += YWeight[i] * c[i];
	//		xyz[2] += ZWeight[i] * c[i];
	//	}
	//	const float
	//		rWeight[3] = { 3.240479f, -1.537150f, -0.498535f };
	//	const float
	//		gWeight[3] = {-0.969256f,  1.875991f,  0.041556f };
	//	const float
	//		bWeight[3] = { 0.055648f, -0.204043f,  1.057311f };
	//float r,g,b;
	//r = xyz[0]*rWeight[0] + xyz[1]*rWeight[1]+ xyz[2] * rWeight[2];
	//g = xyz[0]*gWeight[0] + xyz[1]*gWeight[1]+ xyz[2] * gWeight[2];
	//b = xyz[0]*bWeight[0] + xyz[1]*bWeight[1]+ xyz[2] * bWeight[2];
	return to_int((*spectrum).x) |
		(to_int((*spectrum).y) << 8) |
		(to_int((*spectrum).z) << 16)	;
}
INLINE float spectrum_y(spectrum_t* color)
{
	float y = 0;
	float YWeight[] = {
		0.212671f, 0.715160f, 0.072169f
	};
	y+= YWeight[0] * (*color).x;
	y+= YWeight[1] * (*color).y;
	y+= YWeight[2] * (*color).z;
	return y;	
}

INLINE float* load_spectrum(float* addr,spectrum_t *spectrum)
{
	(*spectrum).x = addr[0];
	(*spectrum).y = addr[1];
	(*spectrum).z = addr[2];
	(*spectrum).w = addr[3];
	return addr+4;
}
#endif