#ifndef MA_VERTEX_HPP
#define MA_VERTEX_HPP

#include "Vector.hpp"
namespace ma{
	//a very simple  vertex type only have position information
	template<typename Vector_t>
	struct MAVertex{
		typedef typename scalar_type<Vector_t>::type scalar_type;
		typedef Vector_t vector_type;
		//
		Vector_t position;
	};
	namespace vertex_op{

	}
}

#endif