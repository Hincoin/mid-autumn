
#include "MAMesh.hpp"

using namespace ma;

typedef MAVertex<ma::vector3f,ma::vector3f> vertex_t;
typedef ma::TriFaceByIndex<> face_idx_t;
struct tmp_material{};
typedef tmp_material material_t;

typedef MARenderMeshFixed<vertex_t,face_idx_t,material_t> mesh_t;


inline void mesh_test()
{
	//mesh_t mesh;
	std::vector<vertex_t> v_s;
	std::vector<face_idx_t> f_s;
	std::vector<material_t> m_s;

	vertex_t v0(0,0,0),v1(1,1,0),v2(2,2,0),v3(3,3,0);
	v_s.push_back(v0);
	v_s.push_back(v1);
	v_s.push_back(v2);
	
	face_idx_t f0(0,1,2),f1(1,2,3);
	
	f_s.push_back(f0);
	f_s.push_back(f1);

	m_s.push_back(material_t());

	mesh_t mesh(v_s,f_s,m_s);

}