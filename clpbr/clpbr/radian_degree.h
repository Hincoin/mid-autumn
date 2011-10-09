#ifndef _RADIAN_DEGREE_H_
#define _RADIAN_DEGREE_H_

struct radian_t{
	float value;
	explicit radian_t(float v):value(v){}
};
struct degree_t{
	float value;
	explicit degree_t(float v):value(v){}
};

inline radian_t degree_to_radian(degree_t deg)
{
	return radian_t(deg.value * 0.0174533f);
}

inline degree_t radian_to_degree(radian_t rad)
{
	return degree_t(rad.value * 180.f/3.1415926f);
}

#endif