#ifndef _TRIANGLE_MESH_OCL_H_
#define _TRIANGLE_MESH_OCL_H_

//OpenCL side code for triangle mesh
#include "geometry.h"
#include "differential_geometry.h"
#include "ray.h"
#include "matrix4x4.h"
#include "mc.h"

typedef struct 
{
	vector3f_t p0,p1,p2;
	vector3f_t n0,n1,n2;
	vector3f_t s0,s1,s2;
	float uv0[2],uv1[2],uv2[2];
	transform_t object_to_world;
	int reverse_orientation;
}triangle_t;

INLINE void load_triangle_data_(GLOBAL float* triangle_mesh_memory, unsigned v0_index,unsigned v1_index, unsigned v2_index,triangle_t *triangle)
{
	unsigned number_triangles = as_uint(triangle_mesh_memory[0]);
	unsigned number_vertex = as_uint(triangle_mesh_memory[1]);
	unsigned reverse_orientation = as_uint(triangle_mesh_memory[2]);
	triangle->reverse_orientation = reverse_orientation;
	unsigned transform_start = 3;
	/*
	transform_init(triangle->object_to_world,
		triangle_mesh_memory[transform_start],triangle_mesh_memory[transform_start+1],triangle_mesh_memory[transform_start+2],triangle_mesh_memory[transform_start+3],
		triangle_mesh_memory[transform_start+4],triangle_mesh_memory[transform_start+5],triangle_mesh_memory[transform_start+6],triangle_mesh_memory[transform_start+7],
		triangle_mesh_memory[transform_start+8],triangle_mesh_memory[transform_start+9],triangle_mesh_memory[transform_start+10],triangle_mesh_memory[transform_start+11],
		triangle_mesh_memory[transform_start+12],triangle_mesh_memory[transform_start+13],triangle_mesh_memory[transform_start+14],triangle_mesh_memory[transform_start+15])
		*/
	unsigned point_start = transform_start+16;//add transform_data
	unsigned normal_start = point_start + 3 * number_vertex;
	unsigned tangent_start = normal_start + 3 * number_vertex;
	unsigned uvs_start = tangent_start + 3 * number_vertex;
	triangle->p0.x = triangle_mesh_memory[3*v0_index + point_start];
	triangle->p0.y = triangle_mesh_memory[3*v0_index + 1 + point_start];
	triangle->p0.z = triangle_mesh_memory[3*v0_index + 2 + point_start];

	triangle->p1.x = triangle_mesh_memory[3*v1_index + point_start];
	triangle->p1.y = triangle_mesh_memory[3*v1_index + 1 + point_start];
	triangle->p1.z = triangle_mesh_memory[3*v1_index + 2 + point_start];

	triangle->p2.x = triangle_mesh_memory[3*v2_index + point_start];
	triangle->p2.y = triangle_mesh_memory[3*v2_index + 1 + point_start];
	triangle->p2.z = triangle_mesh_memory[3*v2_index + 2 + point_start];
	//load normals
	triangle->n0.x = triangle_mesh_memory[3*v0_index + normal_start];
	triangle->n0.y = triangle_mesh_memory[3*v0_index + 1 + normal_start];
	triangle->n0.z = triangle_mesh_memory[3*v0_index + 2 + normal_start];

	triangle->n1.x = triangle_mesh_memory[3*v1_index + normal_start];
	triangle->n1.y = triangle_mesh_memory[3*v1_index + 1 + normal_start];
	triangle->n1.z = triangle_mesh_memory[3*v1_index + 2 + normal_start];

	triangle->n2.x = triangle_mesh_memory[3*v2_index + normal_start];
	triangle->n2.y = triangle_mesh_memory[3*v2_index + 1 + normal_start];
	triangle->n2.z = triangle_mesh_memory[3*v2_index + 2 + normal_start];
	//load tangent
	triangle->s0.x = triangle_mesh_memory[3*v0_index + tangent_start];
	triangle->s0.y = triangle_mesh_memory[3*v0_index + 1 + tangent_start];
	triangle->s0.z = triangle_mesh_memory[3*v0_index + 2 + tangent_start];

	triangle->s1.x = triangle_mesh_memory[3*v1_index + tangent_start];
	triangle->s1.y = triangle_mesh_memory[3*v1_index + 1 + tangent_start];
	triangle->s1.z = triangle_mesh_memory[3*v1_index + 2 + tangent_start];

	triangle->s2.x = triangle_mesh_memory[3*v2_index + tangent_start];
	triangle->s2.y = triangle_mesh_memory[3*v2_index + 1 + tangent_start];
	triangle->s2.z = triangle_mesh_memory[3*v2_index + 2 + tangent_start];
	//load uvs
	triangle->uv0[0] = triangle_mesh_memory[2*v0_index + uvs_start];
	triangle->uv0[1] = triangle_mesh_memory[2*v0_index + 1 + uvs_start];

	triangle->uv1[0] = triangle_mesh_memory[2*v1_index + uvs_start];
	triangle->uv1[1] = triangle_mesh_memory[2*v1_index + 1 + uvs_start];

	triangle->uv2[0] = triangle_mesh_memory[2*v2_index + uvs_start];
	triangle->uv2[1] = triangle_mesh_memory[2*v2_index + 1 + uvs_start];
}
INLINE void load_triangle_vertex8(GLOBAL float* triangle_mesh_data,unsigned triangle_mesh_start,unsigned triangle_index,triangle_t *triangle)
{
	GLOBAL float* triangle_mesh_memory = triangle_mesh_data + triangle_mesh_start;
	unsigned number_triangles = as_uint(triangle_mesh_memory[1]);
	unsigned number_vertex = as_uint(triangle_mesh_memory[1]);
	unsigned transform_start = 3;
	unsigned point_start = transform_start+16;//add transform_data
	unsigned normal_start = point_start + 3 * number_vertex;
	unsigned tangent_start = normal_start + 3 * number_vertex;
	unsigned uvs_start = tangent_start + 3 * number_vertex;

	unsigned vertex_index_start = uvs_start + 2*number_vertex +  (3 * triangle_index) / 4;
	unsigned vertex_index = as_uint(triangle_mesh_memory[vertex_index_start]);
	unsigned vertex_index_next = as_uint(triangle_mesh_memory[vertex_index_start + 1]);

	unsigned char v_index[] = {
		(unsigned char)(vertex_index & 0x000000ff),
		(unsigned char)((vertex_index & 0x0000ff00) >> 8),
		(unsigned char)((vertex_index & 0x00ff0000) >> 16),
		(unsigned char)((vertex_index & 0xff000000) >> 24),
		(unsigned char)((vertex_index_next & 0x000000ff)),
		(unsigned char)((vertex_index_next & 0x0000ff00) >> 8)
	};
	unsigned char v0_index = v_index[(3*triangle_index)%4];
	unsigned char v1_index = v_index[(3*triangle_index)%4+1];
	unsigned char v2_index = v_index[(3*triangle_index)%4+2];
	load_triangle_data_(triangle_mesh_memory,v0_index,v1_index,v2_index,triangle);
}
//bool intersect_triangle(GLOBAL float* triangle_data)
INLINE int intersect_triangleP(const triangle_t *triangle,
					  ray_t *r
					  )
{
	vector3f_t e1,e2;
	vsub(e1,triangle->p1,triangle->p0);
	vsub(e2,triangle->p2,triangle->p0);
	vector3f_t s1;
	vxcross(s1,r->d,e2);
	float divisor = vdot(s1, e1);

	if(divisor == 0.f) return 0;
	float inv_divisor = 1.f/divisor;

	vector3f_t d;
	vsub(d,r->o, triangle->p0);
	float b1 = vdot(d,s1) * inv_divisor;
	if (b1<0 || b1 > 1)
		return 0;

	vector3f_t s2;
	vxcross(s2,d,e1);
	float b2 = vdot(r->d, s2) * inv_divisor;
	if(b2 < 0.f || b1 + b2 > 1.f)
		return 0;

	float t = vdot(e2,s2) * inv_divisor;
	if(t < r->mint || t > r->maxt)
		return 0;

	return 1;
}

INLINE void get_uvs(const triangle_t *triangle,float uvs[3][2])
{
	/*
	uvs[0][0] = triangle->uv0[0];
	uvs[0][1] = triangle->uv0[1];	

	uvs[1][0] = triangle->uv1[0];
	uvs[1][1] = triangle->uv1[1];	

	uvs[2][0] = triangle->uv2[0];
	uvs[2][1] = triangle->uv2[1];	
	*/

	uvs[0][0] = 0.f; uvs[0][1] = 0.f;
	uvs[1][0] = 1.f; uvs[1][1] = 0.f;
	uvs[2][0] = 1.f; uvs[2][1] = 1.f;
}
INLINE int intersect_triangle(const triangle_t *triangle,
					 ray_t *r,
					 float *tHit,
					 differential_geometry_t *dg,
					 float *epsilon)
{
	vector3f_t e1,e2;
	vsub(e1,triangle->p1, triangle->p0);
	vsub(e2,triangle->p2, triangle->p0);
	vector3f_t s1;
	vxcross(s1,r->d, e2);
	float divisor = vdot(s1,e1);

	if (divisor == 0.f)
		return 0;
	float inv_divisor = 1.f/divisor;

	vector3f_t d;
	vsub(d,r->o, triangle->p0);
	float b1 = vdot(d,s1) * inv_divisor;
	if (b1<0 || b1 > 1)
		return 0;

	vector3f_t s2;
	vxcross(s2,d,e1);
	float b2 = vdot(r->d, s2) * inv_divisor;
	if(b2 < 0.f || b1 + b2 > 1.f)
		return 0;

	float t = vdot(e2,s2) * inv_divisor;
	if(t < r->mint || t > r->maxt)
		return false;

	vector3f_t dpdu,dpdv;
	float uvs[3][2];
	get_uvs(triangle,uvs);

	//
	float du1 = uvs[0][0] - uvs[2][0];
	float du2 = uvs[1][0] - uvs[2][0];
	float dv1 = uvs[0][1] - uvs[2][1];
	float dv2 = uvs[1][1] - uvs[2][1];

	vector3f_t dp1,dp2;
	vsub(dp1,triangle->p0, triangle->p2);
	vsub(dp2,triangle->p1, triangle->p2);
	float determinant = du1 * dv2 - dv1*du2;

	if(determinant == 0.f)
	{
		vector3f_t v1;
		vxcross(v1,e2,e1);
		vnorm(v1);
		coordinate_system(&v1,&dpdu,&dpdv);
	}
	else{
		float inv_det = 1.f/ determinant;
		vector3f_t tmp0,tmp1;

		vsmul(tmp0,dv2,dp1);
		vsmul(tmp1,dv1,dp2);
		vsub(dpdu,tmp0,tmp1);
		vsmul(dpdu,inv_det,dpdu);

		vsmul(tmp0,(du2),dp1);
		vsmul(tmp1,(du1),dp2);
		vsub(dpdv,tmp1,tmp0);
		vsmul(dpdv,inv_det,dpdv);
	}
	float b0 = 1-b1-b2;
	float tu = b0 * uvs[0][0] + b1*uvs[1][0] + 2*uvs[2][0];
	float tv = b0 * uvs[0][1] + b1*uvs[1][1] + 2*uvs[2][1];

	//differential_geometry_init(dg,)
	point3f_t p;
	rpos(p,(*r),t);
	vector3f_t dndu,dndv;
	vclr(dndu);
	vclr(dndv);
	differential_geometry_init(*dg,p,dpdu,dpdv,dndu,dndv,tu,tv,triangle);
	*tHit = t;
	*epsilon = 1e-3f * *tHit;
	return 1;
}

INLINE float triangle_area(triangle_t *triangle)
{
	vector3f_t e1,e2;
	vsub(e1,triangle->p1, triangle->p0);
	vsub(e2,triangle->p2, triangle->p0);
	vector3f_t v_norm;
	vxcross(v_norm,e1,e2);
	return 0.5f * sqrt(vdot(v_norm,v_norm));
}

INLINE void triangle_sample(triangle_t* triangle, float u1,float u2, normal3f_t *ns, point3f_t *point)
{
	float b1,b2;
	uniform_sample_triangle(u1,u2,&b1,&b2);

	point3f_t p,p_tmp;
	vsmul(p_tmp,b1,triangle->p0);
	vsmul(p,b2,triangle->p1);
	vadd(p,p,p_tmp);
	vsmul(p_tmp,(1.f-b1-b2),triangle->p2);
	vadd(p,p, p_tmp);
	vassign((*point),p);
	
	vector3f_t e1,e2;
	vsub(e1, triangle->p1,triangle->p0);
	vsub(e2, triangle->p2,triangle->p0);
	vxcross(*ns, e1,e2);
	vnorm(*ns);
	if(triangle->reverse_orientation) vneg(*ns,*ns);
	return;
}

#endif