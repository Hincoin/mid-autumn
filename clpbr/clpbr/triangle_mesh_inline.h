//these are all c++ side code
#include "triangle_array.h"


template<typename IndexType>
void triangle_mesh_init(TriangleMesh<IndexType> *triangle_mesh, size_t number_triangle,size_t number_vertex,int reverse_orientation,
						const transform_t obj2world,float *point_buffer, IndexType *index_buffer,float *normal_buffer,float *tangent_buffer,float *uv_buffer)
{
	triangle_mesh->number_triangle = number_triangle;
	triangle_mesh->number_vertex = number_vertex;
	triangle_mesh->reverse_orientation = reverse_orientation;
	triangle_mesh->object_to_world = obj2world;
	const int point_memory_size = number_vertex * 3 * sizeof(triangle->point_buffer[0]);
	const int index_memory_size = number_triangle * 3 * sizeof(triangle->vertex_index_buffer[0]);
	triangle_mesh->point_buffer	= (float*)::malloc(point_memory_size);
	::memcpy(triangle_mesh->point_buffer,point_buffer,point_memory_size);
	
	triangle_mesh->vertex_index_buffer = (size_t*)::malloc(index_memory_size);
	::memcpy(triangle_mesh->vertex_index_buffer,index_buffer,index_memory_size);

	tiangle_mesh->normal_buffer = (float*)::malloc(point_memory_size);
	::memcpy(triangle_mesh->normal_buffer,normal_buffer,point_memory_size);

	triangle_mesh->tangent_buffer = (float*)::malloc(point_memory_size);
	::memcpy(triangle_mesh->tangent_buffer,tangent_buffer,point_memory_size);

	const int uvs_buffer_size = 2 * number_vertex * sizeof(triangle_uvs_buffer[0]);
	triangle_mesh->uvs_buffer = (float*)::malloc(uvs_buffer_size);
	::memcpy(triangle_mesh->uvs_buffer,tangent_buffer,uvs_buffer_size);
}
template<typename IndexType>
void triangle_mesh_destroy(TriangleMesh<IndexType> *triangle_mesh)
{
	::free(triangle_mesh->point_buffer);
	::free(triangle_mesh->vertex_index_buffer);
	::free(triangle_mesh->normal_buffer);
	::free(triangle_mesh->tangent_buffer);
	::free(triangle_mesh->uvs_buffer);
}
//functions
template<typename IndexType>
bbox_t world_bound(const TriangleMesh<IndexType>& triangle_mesh)
{
	bbox_t bounds;
	for(size_t i = 0;i < triangle_mesh.number_vertex; ++i)
	{
		point3f_t p;
		p.x = triangle_mesh.point_buffer[i*3];
		p.y = triangle_mesh.point_buffer[i*3+1];
		p.z = triangle_mesh.point_buffer[i*3+2];
		bbox_union_with_point(&bounds,p);
	}
	return bounds;
}

template<typename VertexIndexType,typename TriangleIndexType>
bbox_t world_bound(const TriangleMesh<VertexIndexType>& triangle_mesh,const Triangle<VertexIndexType>& triangle)
{
	point3f_t v0,v1,v2;
	v0.x = triangle_mesh.point_buffer[triangle.vertex_index*3];
	v0.y = triangle_mesh.point_buffer[triangle.vertex_index*3+1];
	v0.z = triangle_mesh.point_buffer[triangle.vertex_index*3+2];

	v1.x = triangle_mesh.point_buffer[(triangle.vertex_index+1)*3];
	v1.y = triangle_mesh.point_buffer[(triangle.vertex_index+1)*3+1];
	v1.z = triangle_mesh.point_buffer[(triangle.vertex_index+1)*3+2];

	v2.x = triangle_mesh.point_buffer[(triangle.vertex_index+2)*3];
	v2.y = triangle_mesh.point_buffer[(triangle.vertex_index+2)*3+1];
	v2.z = triangle_mesh.point_buffer[(triangle.vertex_index+2)*3+2];
	bbox_t bounds;
	bbox_union_with_point(bounds,v0);
	bbox_union_with_point(bounds,v1);
	bbox_union_with_point(bounds,v2);
	return bounds;
}

