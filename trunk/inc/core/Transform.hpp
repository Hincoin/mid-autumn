#ifndef MA_INCLUDED_TRANSFORM_HPP
#define MA_INCLUDED_TRANSFORM_HPP

#include "EigenLib.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"

namespace ma{
	template<typename T,int Dim,typename Impl = Eigen::Transform<T,Dim> >
	class Transform:public Impl{
		typedef typename matrix_type<T,Dim+1,Dim+1>::type MatrixType;
		typedef typename vector_type<T,Dim>::type VectorType;
	public:
		typedef Impl parent_type;
		Transform(const Impl& im):Impl(im){}
		Transform(move_from<Impl> src){self().matrix().swap(src.source.matrix());}
		Transform(){}
		Transform(const Transform& other):Impl(other.self()){};
		Transform(move_from<Transform> src){self().matrix().swap(src.source.matrix());};
		template<typename Other>
		Transform(const Other& other):Impl(other){};
		template<typename Other>
		Transform(move_from<Other> o){self().matrix().swap(o.source.matrix());}

		explicit Transform(const MatrixType& m){
			self().matrix() = m.self();
		};
		explicit Transform(move_from<MatrixType> m)
		{
			self().matrix().swap(m.source.self());
		}

		Transform(T m[Dim+1][Dim+1]){self().matrix().swap(MatrixType(m).self());}
		//Transform(T m[(Dim+1)*(Dim+1)]){self().matrix().swap(MatrixType(m).self());}

		Transform& operator=(Transform other){this->swap(other);return *this;}

		void swap(Transform& other){self().matrix().swap(other.self().matrix());}
		Transform inverse(Eigen::TransformTraits t = Eigen::Projective)const{return Transform(move_from<MatrixType>(self().inverse(t)));}
		Transform transpose()const{return Transform(move_from<Impl>(self().transpose()));}
		Transform& scale(const VectorType& v){self().scale(v);return *this;}
		Transform& translate(const VectorType& v){self().translate(v);return *this;};
		inline Transform& rotate(T radian,const VectorType& axis){self().rotate(make_angle_axis(radian,axis));return *this;}
		Transform& identity(){self().setIdentity();return *this;}
		//Transform operator*(const Transform& tt)const{return (self()*tt.self());}
		Transform& operator*=(const Transform& other)
		{self() = self() * other.self(); return *this;}

		operator Impl&(){return static_cast<Impl&>(*this);}
		operator const Impl&()const{return static_cast<const Impl&>(*this);}
	private:
		Impl& self(){return *this;}
		const Impl& self()const{return *this;}
	};
}

namespace ma{
	template<typename T,int Dim>
	struct transform_type{
		typedef ma::Transform<T,Dim> type;
		static const int dimension = Dim;
	};


	namespace transform_op{
		///! transformation
		template<typename Tran>
		bool swap_handness(const Tran& m){
			//return ((m.matrix()[0][0] *
			//	(m.matrix()[1][1] * m.matrix()[2][2] -
			//	m.matrix()[1][2] * m.matrix()[2][1])) -
			//	(m.matrix()[0][1] *
			//	(m.matrix()[1][0] * m.matrix()[2][2] -
			//	m.matrix()[1][2] * m.matrix()[2][0]))+
			//	(m.matrix()[0][2] *
			//	(m.matrix()[1][0] * m.matrix()[2][1] -
			//	m.matrix()[1][1] * m.matrix()[2][0]))) < 0;
			return ((m.matrix()(0,0) *
				(m.matrix()(1,1) * m.matrix()(2,2) -
				m.matrix()(1,2) * m.matrix()(2,1))) -
				(m.matrix()(0,1) *
				(m.matrix()(1,0) * m.matrix()(2,2) -
				m.matrix()(1,2) * m.matrix()(2,0)))+
				(m.matrix()(0,2) *
				(m.matrix()(1,0) * m.matrix()(2,1) -
				m.matrix()(1,1) * m.matrix()(2,0)))) < 0;

		}
		template<typename T>
		typename transform_type<T,3>::type lookat(
			T ex,
			T ey,
			T ez,
			T lx,
			T ly,
			T lz,
			T ux,
			T uy,
			T uz)
		{
			typedef typename transform_type<T,3>::type trans_t;
			typedef typename vector_type<T,3>::type vector_t;
			typedef typename matrix_type<T,4,4>::type matrix_t;
			matrix_t m;
			// Initialize fourth column of viewing matrix
			m(0,3) = ex;
			m(1,3) = ey;
			m(2,3) = ez;
			m(3,3) = 1;
			// Initialize first three columns of viewing matrix
			vector_t dir(lx-ex,ly-ey,lz-ez);
			dir.normalize();
			//left or right
			vector_t right(ux,uy,uz);
			right= dir.cross(normalize(right));
			vector_t newUp;
			newUp = right.cross(dir);
			m(0,0) = right.x();
			m(1,0) = right.y();
			m(2,0) = right.z();
			m(3,0) = 0.;
			m(0,1) = newUp.x();
			m(1,1) = newUp.y();
			m(2,1) = newUp.z();
			m(3,1) = 0.;
			m(0,2) = dir.x();
			m(1,2) = dir.y();
			m(2,2) = dir.z();
			m(3,2) = 0.;
			matrix_t tm(m);
			return trans_t(move_from<matrix_t>(tm.inverse()));

			//vector_t eye_pos(ex,ey,ez);
			//vector_t center_pos(lx,ly,lz);
			//vector_t up_dir(ux,uy,uz);
			//const vector_t forward = ((center_pos.self() - eye_pos.self()).normalized());//(center - eye);
			//const vector_t side = ((forward.cross(up_dir)).normalized());//normalize(cross(forward, up));

			//const vector_t up2 = side.cross(forward);//cross(side, forward);

			//trans_t m ;//= transform3f::Identity();
			//m.setIdentity();

			//m(0,0) = side[0];
			//m(0,1) = side[1];
			//m(0,2) = side[2];

			//m(1,0) = up2[0];
			//m(1,1) = up2[1];
			//m(1,2) = up2[2];

			//m(2,0) = -forward[0];
			//m(2,1) = -forward[1];
			//m(2,2) = -forward[2];

			//return m.translate(-eye_pos);
		}
	}


	template<typename T>
	inline typename transform_type<T,3>::type
	 orthographic(T znear, T zfar) {
		//return Scale(1.f, 1.f, 1.f / (zfar-znear)) *
		//	Translate(Vector(0.f, 0.f, -znear));
	}
	template<typename T>
	inline typename transform_type<T,3>::type perspective(T fov, T n, T f) {
			// Perform projective divide
		typedef typename vector_type<T,3>::type vector_t;
		typedef  typename transform_type<T,3>::type transform_t;
		typedef typename matrix_type<T,4>::type matrix_t;
			T inv_denom = reciprocal(f-n);
			matrix_t mat44 ;
			mat44<<1, 0,       0,          0,
				0, 1,       0,          0,
				0, 0, f*inv_denom, -f*n*inv_denom,
				0, 0,       1,          0;
			//T mat44[]={};
			T invTanAng = reciprocal(std::tan(Radians(fov)/2));
			return transform_t(mat44).prescale(vector_t(invTanAng,invTanAng,1));
			// Scale to canonical viewing volume
			//float invTanAng = 1.f / tanf(Radians(fov) / 2.f);
			//return Scale(invTanAng, invTanAng, 1) *
			//	Transform(persp);
	}


	typedef transform_type<float,3>::type transform3f;
}

#include "Point.hpp"
#include "Vector.hpp"

namespace  ma{

	namespace details{
		//template<typename S,int D,typename T>
		//struct transformer_impl;
		//template<typename S,int D>
		//struct transformer_impl<S,D,typename vector_type<S,D>::type>{
		//	typedef typename transform_type<S,D>::type transform_t;
		//	typedef typename vector_type<S,D>::type result_type;
		//	typedef result_type param_type;
		//	static result_type run(const transform_t& trans, const param_type& v)
		//	{
		//		return trans.linear() * v;
		//	}
		//};
		//template<typename S,int D>
		//struct transformer_impl<S,D,typename point_type<S,D>::type>{
		//	typedef typename transform_type<S,D>::type transform_t;
		//	typedef typename point_type<S,D>::type result_type;
		//	typedef result_type param_type;
		//	static result_type run(const transform_t& trans, const param_type& p)
		//	{
		//		typedef typename transform_t::parent_type parent_type;
		//		return result_type(static_cast<const parent_type&>(trans) * p.p);
		//	}
		//};
		//template<typename S,int D>
		//struct transformer_impl<S,D,typename normal_type<S,D>::type>{
		//	typedef typename transform_type<S,D>::type transform_t;
		//	typedef typename normal_type<S,D>::type result_type;
		//	typedef result_type param_type;
		//	static result_type run(const transform_t& trans, const param_type& n)
		//	{
		//		typedef typename transform_t::parent_type parent_type;
		//		typedef typename result_type::parent_type normal_parent;
		//		return /*static_cast<const parent_type&>*/(trans.linear().inverse().transpose()) *
		//			static_cast<const normal_parent&>(n);
		//	}
		//};
		//template<typename S,int D>
		//struct transformer_impl<S,D,typename transform_type<S,D>::type>{
		//	typedef typename transform_type<S,D>::type transform_t;
		//	typedef typename transform_type<S,D>::type result_type;
		//	typedef result_type param_type;
		//	static result_type run(const transform_t& trans, const transform_t& t)
		//	{
		//		typedef typename result_type::parent_type parent_type;
		//		return move_from<result_type>(result_type(static_cast<const parent_type&> (trans) * static_cast<const parent_type&>(t)));
		//	}
		//};
	}
	template<typename S,int D,typename Impl,typename TImpl>
	inline typename vector_type<S,D>::type operator*(const Transform<S,D,TImpl>& trans,
		const Eigen::Matrix<S,D,1>& v)
	{
		return vector_type<S,D>::type(trans.linear() * v);
	}
	template<typename S,int D,typename Impl,typename TImpl>
	inline typename normal_type<S,D>::type operator*(const Transform<S,D,TImpl>& trans,
		const Normal<S,D,Impl>& n)
	{
		typedef typename Normal<S,D,Impl>::parent_type normal_parent;
		return typename normal_type<S,D>::type(trans.linear().inverse().transpose() * static_cast<const normal_parent&>(n));
	}
	template<typename V>
	inline Point<V> operator*(const typename transform_type<typename scalar_type<V>::type,dimensions<V>::value>::type& trans,
		const Point<V>& v)
	{
		return Point<V>(trans * v.p);
	}

	//template<typename S,int D,typename TransImpl,typename Rhs>
	//inline Rhs
	//	operator*(const typename Transform<S,D,TransImpl>& trans,
	//	const Rhs& rhs)
	//{
	//	return details::transformer_impl<S,D,Rhs>::run(trans,rhs);
	//}

}
#endif
