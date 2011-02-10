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
typedef struct  
{
	float x,y,z;
}spectrum_t;

#define color_is_black(_x) ((_x).x == 0.f && (_x).y == 0.f && (_x).z == 0.f)

#define color_clamp(_x,_low,_high) {clamp((_x).x,(_low),(_high));clamp((_x).y,(_low),(_high));clamp((_x).z,(_low),(_high));};

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
	float r = spectrum->x;
	float g = spectrum->y;
	float b = spectrum->z;
	return to_int(r) |
		(to_int(g) << 8) |
		(to_int(b) << 16)	;
}
INLINE unsigned load_color(GLOBAL float *mem,spectrum_t *ret)
{
	vinit(*ret,mem[0],mem[1],mem[2]);
	return 3;
}
#endif