#ifndef _TRIANGLE_ARRAY_H_
#define _TRIANGLE_ARRAY_H_

#include <CL/cl.h>
#include <vector>

struct TriangleArray{
	std::vector<cl_uchar> triangle_vertex8_index;
	std::vector<cl_ushort> triangle_vertex16_index;
	std::vector<cl_uint> triangle_vertex32_index;
};

#endif
