#ifndef _MA_INCLUDED_MATRIX_HPP_
#define _MA_INCLUDED_MATRIX_HPP_

#include "EigenLib.hpp"
#include "VectorType.hpp"
#include "Move.hpp"
#include "Vector.hpp"

namespace ma{
	//adaptor
	template<typename T,int Row,int Col,typename Impl = Eigen::Matrix<T,Row,Col> >
	class Matrix:public Impl{
	public:
		typedef T ScalarType;
		typedef Impl BaseType;
		Matrix(){}
		Matrix(const Matrix& other):BaseType(other.self()){}
		Matrix(move_from<Matrix> other){swap(other.source.self());};
		Matrix(T m_[Row][Col]):BaseType(m_){};
		//Matrix(T t[Row*Col]):BaseType(t){}
		Matrix(const Impl& im):BaseType(im){}
		Matrix(move_from<Impl> im){self().swap(im.source);}
		template<typename OtherImpl>
		Matrix(const OtherImpl& other):BaseType(other){}
		template<typename OtherImpl>
		Matrix(move_from<OtherImpl> o){self().swap(o.source);}

		void swap(Matrix& other){self().swap(other.self());}
		
		Matrix& operator=(Matrix rhs){this->swap(rhs);}
		Impl& self(){return *this;}
		const Impl& self()const{return *this;}

		//operator Impl& (){return static_cast<Impl&>(*this);}
		//operator const Impl& ()const{return static_cast<const Impl&>(*this);}

};
}


namespace ma{
	template<typename T,int Row,int Col=Row>
	struct matrix_type{
		typedef Matrix<T,Row,Col>  type;
		static const int row = Row;
		static const int col = Col;
	};

	typedef matrix_type<int,2,2>::type matrix22i;
	typedef matrix_type<int,3,3>::type matrix33i;
	typedef matrix_type<int,4,4>::type matrix44i;

	typedef matrix_type<float,2,2>::type matrix22f;
	typedef matrix_type<float,3,3>::type matrix33f;
	typedef matrix_type<float,4,4>::type matrix44f;
}
#endif