#ifndef _MA_VECTOR_TYPE_HPP_
#define _MA_VECTOR_TYPE_HPP_

//meta-function to compute the vector type

namespace ma{
	//////////////////////////////////////////////////////////////////////////
	template<typename _Scalar,int _Dim>
	struct vector_type;

	template<typename _S,int _D>
	struct transform_type;

	template<typename _S,int _R,int _C>
	struct matrix_type;

	template<typename S,int N>
	struct normal_type;


	//meta-function to get the scalar type of a vector
	template<typename T> struct dimensions;
}
#endif