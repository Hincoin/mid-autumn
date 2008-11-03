#ifndef MA_TRIANGLE_HPP
#define MA_TRIANGLE_HPP

namespace ma{

enum MA_TRIANGLE_DIR{MA_CLOCK_WISE = 0,MA_ANTI_CLOCK_WISE};

template<typename Vertex_t,MA_TRIANGLE_DIR dir = MA_ANTI_CLOCK_WISE>
struct MATriangle{
	typedef Vertex_t vertex_type;
	
	Vertex_t verts[3];
};

namespace triangle_op{
	//normal

	//
}
}
#endif