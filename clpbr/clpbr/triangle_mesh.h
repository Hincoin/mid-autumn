#ifndef _TRIANGLE_MESH_H_
#define _TRIANGLE_MESH_H_

#include "geometry.h"
//IndexType can be customized to be:
//uint8_t, uint16_t, uint32_t
template<typename IndexType>
struct TriangleMesh{
	size_t number_triangle,number_vertex;
	int reverse_orientation;
	transform_t object_to_world;
	float *point_buffer;//size:3*number_vertex
	float *normal_buffer;//size:3*number_vertex
	float *tangent_buffer;//size:3*number_vertex;data for shading
	float *uvs_buffer;//size:2*number_vertex
	IndexType *vertex_index_buffer;//size: number_vertex
};



template<typename VertexIndexType>
struct Triangle{
	Triangle( VertexIndexType vert_idx):vertex_index(vert_idx){}
	VertexIndexType vertex_index;
};

template<typename IndexType>
struct TriangleTraits{
	enum{triangle_type = sizeof(IndexType)/sizeof(char)};
};

#include "triangle_mesh_inline.h"

#endif