////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008 by luozhiyuan (luozhiyuan@gmail.com)
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author makes no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////


#ifndef VECTOR_HPP
#define VECTOR_HPP



// the following implementation is not conformed to c++ standard when using nameless union
//
//#include <boost/static_assert.hpp>
//#include <boost/type_traits.hpp>
//#include <boost/mpl/if.hpp>
//#include <cstdlib>
//#include <boost/preprocessor.hpp>
//
//
//#define MA_MAX_PARAMETERS 20
//
//namespace ma
//{
//	//vector concept requirements:
//	//all arithmetic operation here should provided for T
//	namespace core{
//
//#define MA_VECTOR_CONSTRUCTOR(z,N_P,data) \
//	template<typename T_Self> \
//		Vector( BOOST_PP_ENUM_PARAMS( BOOST_PP_ADD(N_P,1), T_Self param) )\
//		:Storage (BOOST_PP_ENUM_PARAMS( BOOST_PP_ADD(N_P,1), param))\
//		{																		\
//		BOOST_STATIC_ASSERT((boost::is_convertible<T_Self,T>::value));			\
//		BOOST_STATIC_ASSERT(size == N_P + 1);\
//		}
//
//#define MA_VECTOR_CONSTRUCTORS(M)\
//		BOOST_PP_REPEAT(M,MA_VECTOR_CONSTRUCTOR,_)
//
//
//
//		template<typename T>
//		struct ParameterType{
//			struct null_type;
//			template <class U> struct AddParameterType
//			{
//				typedef const U & type;
//			};
//
//			template <class U> struct AddParameterType<U &>
//			{
//				typedef U & type;
//			};
//
//			template <> struct AddParameterType<void>
//			{
//				typedef null_type type;
//			};
//			static const bool is_add_param = boost::is_arithmetic<T>::value || 
//				boost::is_pointer<T>::value;
//			typedef typename boost::mpl::if_c<is_add_param, typename AddParameterType<T>::type,T>::type type;
//		};
//		template<typename T, typename Storage>
//		struct Vector:Storage{
//		private:
//		public:
//			typedef T scalar_type;
//			typedef std::size_t size_type;
//			typedef Storage storage_type;
//			typedef typename ParameterType<T>::type parameter_type;
//		private:
//			void copy(const T* src,boost::true_type )
//			{
//				std::memcpy(val,src,size);
//			}
//			void copy(const T* src,boost::false_type)
//			{
//				for (size_type i = 0 ;i < size ; ++i)
//				{
//					val[i] = src[i];
//				}
//			}
//			template<typename T_From>
//			void copy(const T_From* src)
//			{
//				for (size_type i = 0 ;i < size ; ++i)
//				{
//					val[i] = static_cast<T>(src[i]);
//				}
//			}
//
//		public:
//			Vector(){
//			}
//			Vector(const Vector& other){
//				copy(other.val,boost::has_trivial_copy<T>());
//			}
//			Vector(T v[storage_type::size]){
//				copy(v,boost::has_trivial_copy<T>());
//			}
//			MA_VECTOR_CONSTRUCTORS(MA_MAX_PARAMETERS)
//
//			template<typename T_Other,typename Storage_Other>
//			explicit Vector(const Vector<T_Other,Storage_Other>& other){
//				BOOST_STATIC_ASSERT(storage_type::size <= Storage_Other::size);
//				BOOST_STATIC_ASSERT((boost::is_convertible<T_Other,scalar_type>::value));
//				copy(other.val);
//			}
//			T& operator[](std::size_t idx){return val[idx];}
//			const T& operator[](std::size_t idx)const{return val[idx];}
//
//			Vector& operator=(const Vector& other){ copy(other.val,boost::has_trivial_copy<T>());return *this;}
//			//smart compiler will unroll these loop
//			Vector& operator+=(const Vector& other);//{for(size_type i = 0;i < size;++i)val[i] += other[i];return *this;}
//			Vector& operator-=(const Vector& other);//{for(size_type i = 0;i < size;++i)val[i] -= other[i];return *this;}
//			Vector& operator*=(parameter_type s);//{for(size_type i = 0;i < size;++i)val[i] *= s;return *this;}
//			Vector& operator/=(parameter_type s);//{for(size_type i = 0;i < size;++i)val[i] /= s;return *this;}
//		};
//		namespace meta_details{
//
//#define  MA_VV_BINARAY_EQUAL_UNROLLING(OPNAME) \
//			template<std::size_t N,typename T,typename Storage> \
//			struct OPNAME##binary_equal{ \
//				typedef Vector<T,Storage> vector_type;	\
//				static vector_type& execute(vector_type& lhs,const vector_type& rhs)\
//				{\
//					lhs[N] OP rhs[N];\
//					return OPNAME##binary_equal<N-1,T,Storage>::execute(lhs,rhs);\
//				}\
//			};\
//			template<typename T,typename Storage>\
//			struct OPNAME##binary_equal<0,T,Storage>{\
//				typedef Vector<T,Storage> vector_type;\
//				static vector_type& execute(vector_type& lhs,const vector_type& rhs)\
//				{\
//					lhs[0] OP rhs[0];\
//					return lhs;\
//				}\
//			};
//#define MA_VS_BINARAY_EQUAL_UNROLLING(OPNAME) \
//	template<std::size_t N,typename T,typename Storage> \
//			struct OPNAME##binary_equal{ \
//			typedef Vector<T,Storage> vector_type;	\
//			static vector_type& execute(vector_type& lhs,typename vector_type::parameter_type rhs)\
//			{\
//			lhs[N] OP rhs;\
//			return OPNAME##binary_equal<N-1,T,Storage>::execute(lhs,rhs);\
//		}\
//			};\
//			template<typename T,typename Storage>\
//			struct OPNAME##binary_equal<0,T,Storage>{\
//			typedef Vector<T,Storage> vector_type;\
//			static vector_type& execute(vector_type& lhs,typename vector_type::parameter_type rhs)\
//			{\
//			lhs[0] OP rhs;\
//			return lhs;\
//			}\
//			};
//
//
//#define MA_VS_BINARAY_UNROLLING(OPNAME) \
//	template<typename T,typename Storage>\
//	inline Vector<T,Storage> OPNAME##binary(typename Vector<T,Storage>::parameter_type s,const Vector<T,Storage>& v,boost::mpl::int_<2>)\
//			{\
//			return Vector<T,Storage>( v[0] OP s, v[1] OP s);\
//		}\
//		template<typename T,typename Storage>\
//		inline Vector<T,Storage> OPNAME##binary(typename Vector<T,Storage>::parameter_type s,const Vector<T,Storage>& v,boost::mpl::int_<3>)\
//			{\
//			return Vector<T,Storage>( v[0] OP s, v[1] OP s,v[2] OP s);\
//		}\
//		template<typename T,typename Storage>\
//		inline Vector<T,Storage> OPNAME##binary(typename Vector<T,Storage>::parameter_type s,const Vector<T,Storage>& v,boost::mpl::int_<4>)\
//			{\
//			return Vector<T,Storage>( v[0] OP s, v[1] OP s,v[2] OP s, v[3] OP s);\
//		}\
//		template<typename T,typename Storage>\
//		inline Vector<T,Storage> OPNAME##binary(typename Vector<T,Storage>::parameter_type s,const Vector<T,Storage>& v,boost::mpl::int_<-1>)\
//			{\
//			Vector<T,Storage> result;\
//			for (std::size_t i = 0;i < Vector<T,Storage>::size; ++i)\
//			{\
//			result[i] =  v[i] OP s;\
//		}\
//		}
//
//#define MA_VV_BINARAY_UNROLLING(OPNAME) \
//	template<typename T,typename Storage>\
//	inline Vector<T,Storage> OPNAME##binary(const Vector<T,Storage>& lhs,const Vector<T,Storage>& rhs,boost::mpl::int_<2>)\
//			{\
//			return Vector<T,Storage>(lhs[0] OP rhs[0],lhs[1] OP rhs[1]);\
//		}\
//		template<typename T,typename Storage>\
//		inline Vector<T,Storage> OPNAME##binary(const Vector<T,Storage>& lhs,const Vector<T,Storage>& rhs,boost::mpl::int_<3>)\
//			{\
//			return Vector<T,Storage>(lhs[0] OP rhs[0],lhs[1] OP rhs[1], lhs[2] OP rhs[2]);\
//		}\
//		template<typename T,typename Storage>\
//		inline Vector<T,Storage> OPNAME##binary(const Vector<T,Storage>& lhs,const Vector<T,Storage>& rhs,boost::mpl::int_<4>)\
//			{\
//			return Vector<T,Storage>(lhs[0] OP rhs[0],lhs[1] OP rhs[1], lhs[2] OP rhs[2],lhs[3]  OP rhs[3]);\
//		}\
//		template<typename T,typename Storage>\
//		inline Vector<T,Storage> OPNAME##binary(const Vector<T,Storage>& lhs,const Vector<T,Storage>& rhs,boost::mpl::int_<-1>)\
//			{\
//			Vector<T,Storage> result;\
//			for (std::size_t i = 0;i < Vector<T,Storage>::size; ++i)\
//			{\
//			result[i] = lhs[i] OP rhs[i];\
//		}\
//		return result;\
//		}
//
//#define OP +=
//MA_VV_BINARAY_EQUAL_UNROLLING(add_)
//#undef  OP
//#define OP -=
//MA_VV_BINARAY_EQUAL_UNROLLING(sub_)
//#undef OP
//
//#define OP *=
//MA_VS_BINARAY_EQUAL_UNROLLING(mul_)
//#undef  OP
//
//#define OP /=
//MA_VS_BINARAY_EQUAL_UNROLLING(div_)
//#undef OP
//
//#define OP +
//MA_VV_BINARAY_UNROLLING(add_)
//#undef OP
//
//#define OP -
//MA_VV_BINARAY_UNROLLING(sub_)
//#undef OP
//
//#define OP *
//MA_VS_BINARAY_UNROLLING(mul_)
//#undef OP
//#define OP /
//MA_VS_BINARAY_UNROLLING(div_)
//
//
//
//			template<std::size_t N,typename T,typename Storage>
//			struct dot_loop_unroll{
//				typedef typename Vector<T,Storage>::scalar_type result_type;
//
//				static result_type execute(const Vector<T,Storage>& vlhs,const Vector<T,Storage>& vrhs)
//				{
//					BOOST_STATIC_ASSERT( (N < Vector<T,Storage>::size) );
//					return vlhs[N]*vrhs[N] + dot_loop_unroll<N-1,T,Storage>::execute(vlhs,vrhs);
//				}
//			};
//			template<typename T,typename Storage>
//			struct dot_loop_unroll<0,T,Storage>{
//				typedef typename Vector<T,Storage>::scalar_type result_type;
//
//				static result_type execute(const Vector<T,Storage>& vlhs,const Vector<T,Storage>& vrhs)
//				{
//					BOOST_STATIC_ASSERT( (0 < Vector<T,Storage>::size) );
//					return vlhs[0]*vrhs[0];
//				}
//			};
//
//
//
//		}
//		template<typename T,typename Storage>
//		inline typename Vector<T,Storage>::scalar_type
//			dot(const Vector<T,Storage>& vlhs,const Vector<T,Storage>& vrhs)
//		{
//			typedef Vector<T,Storage> vector_type;
//			return meta_details::dot_loop_unroll<vector_type::size - 1,T,Storage>::execute(vlhs,vrhs);
//		}
//
//		//binary-cross operation for 3d vector
//		//only for 3d vectors
//		template<typename T,typename Storage>
//		inline Vector<T,Storage>
//			cross(const Vector<T,Storage>& vlhs,const Vector<T,Storage>& vrhs)
//		{
//			typedef Vector<T,Storage> vector_type;
//			BOOST_STATIC_ASSERT((vector_type::size == 3));
//			return Vector<T,Storage>(
//				vlhs[1]*vrhs[2] - vlhs[2] * vrhs[1],
//				vlhs[2]*vrhs[0] - vlhs[0] * vrhs[2],
//				vlhs[0]*vrhs[1] - vlhs[1] * vrhs[0]
//				);
//		}
//		template<typename T,typename Storage>
//		inline Vector<T,Storage>
//			operator *(typename Vector<T,Storage>::parameter_type s,const Vector<T,Storage>& v)
//		{
//			typedef Vector<T,Storage> vector_type;
//			typedef typename 
//				boost::mpl::if_c<
//				(vector_type::size > 1 && vector_type::size < 5),
//				boost::mpl::int_<vector_type::size>,
//				boost::mpl::int_<-1> >::type selected_type;
//
//			return meta_details::mul_binary(s,v,selected_type());
//		}
//		template<typename T,typename Storage>
//		inline Vector<T,Storage>
//			operator *(const Vector<T,Storage>& v,typename Vector<T,Storage>::parameter_type s)
//		{
//			return s * v;
//		}
//
//		template<typename T,typename Storage>
//		inline Vector<T,Storage>
//			operator+(const Vector<T,Storage>& lhs,const Vector<T,Storage>& rhs)
//		{
//			typedef Vector<T,Storage> vector_type;
//			typedef typename 
//				boost::mpl::if_c<
//				(vector_type::size > 1 && vector_type::size < 5),
//				boost::mpl::int_<vector_type::size>,
//				boost::mpl::int_<-1> >::type selected_type;
//
//			return meta_details::add_binary(lhs,rhs,selected_type());
//		}
//
//		template<typename T,typename Storage>
//		inline Vector<T,Storage>
//			operator-(const Vector<T,Storage>& lhs,const Vector<T,Storage>& rhs)
//		{
//			typedef Vector<T,Storage> vector_type;
//			typedef typename 
//				boost::mpl::if_c<
//				(vector_type::size > 1 && vector_type::size < 5),
//				boost::mpl::int_<vector_type::size>,
//				boost::mpl::int_<-1> >::type selected_type;
//
//			return meta_details::sub_binary(lhs,rhs,selected_type());
//		}
//
//		template<typename T,typename Storage>
//		inline Vector<T,Storage>
//			operator/(const Vector<T,Storage>& lhs,typename Vector<T,Storage>::parameter_type s)
//		{
//			typedef Vector<T,Storage> vector_type;
//			typedef typename 
//				boost::mpl::if_c<
//				(vector_type::size > 1 && vector_type::size < 5),
//				boost::mpl::int_<vector_type::size>,
//				boost::mpl::int_<-1> >::type selected_type;
//
//			return meta_details::div_binary(s,lhs,selected_type());
//		}
//
//		template<typename T,typename Storage>
//		Vector<T,Storage>& Vector<T,Storage>::operator+=(const Vector<T,Storage>& other)
//		{
//			return meta_details::add_binary_equal<size-1,T,Storage>::execute(*this,other);
//		}
//		template<typename T,typename Storage>
//		Vector<T,Storage>& Vector<T,Storage>::operator-=(const Vector<T,Storage>& other)
//		{
//			return meta_details::sub_binary_equal<size-1,T,Storage>::execute(*this,other);
//		}
//		template<typename T,typename Storage>
//		Vector<T,Storage>& Vector<T,Storage>::operator*=(typename Vector<T,Storage>::parameter_type s)
//		{
//			return meta_details::mul_binary_equal<size-1,T,Storage>::execute(*this,s);
//		}
//		template<typename T,typename Storage>
//		Vector<T,Storage>& Vector<T,Storage>::operator/=(typename Vector<T,Storage>::parameter_type s)
//		{
//			return meta_details::div_binary_equal<size-1,T,Storage>::execute(*this,s);
//		}	
//	}
//}
//
//
//namespace ma{
//	namespace core{
//
//#define MA_STORAGE_ASSIGNMENT(z,N_P,data)\
//	val[N_P] = BOOST_PP_CAT(param,N_P);
//
//#define MA_STORAGE_CONSTRUCTOR(z,N_P,data)\
//	VectorStorageN(BOOST_PP_ENUM_PARAMS( BOOST_PP_ADD(N_P,1), parameter_type param))\
//		{\
//		BOOST_PP_REPEAT(N_P,MA_STORAGE_ASSIGNMENT,N_P)\
//		}
//#define MA_STORAGE_CONSTRUCTORS(M)\
//	BOOST_PP_REPEAT(M,MA_STORAGE_CONSTRUCTOR,_)
//
//		template<typename T,std::size_t N>
//		struct VectorStorageN{
//			static const std::size_t size = N;
//			typedef typename ParameterType<T>::type parameter_type;
//
//			T val[size];
//
//			MA_STORAGE_CONSTRUCTORS(MA_MAX_PARAMETERS)
//		protected:
//			void init(boost::true_type)
//			{
//				std::memset(val,0,size);
//			}
//			void init(boost::false_type)
//			{}
//			VectorStorageN(){init(boost::has_trivial_constructor<T>());}
//		};
//
//		template<typename T>
//		struct VectorStorageN<T,2>{
//			static const std::size_t size = 2;
//			typedef typename ParameterType<T>::type parameter_type;
//			VectorStorageN(parameter_type x_,parameter_type y_)
//				:x(x_),y(y_){}
//
//			union{
//				struct {T x,y;};
//				T val[2];
//			};
//			
//
//		protected:
//			VectorStorageN(){std::memset(this,0,size);}
//		};
//		template<typename T>
//		struct VectorStorageN<T,3>{
//			static const std::size_t size = 3;
//			typedef typename ParameterType<T>::type parameter_type;
//			VectorStorageN(parameter_type x_,parameter_type y_,parameter_type z_)
//				:x(x_),y(y_),z(z_){}
//			union{
//				struct {T x,y,z;};
//				T val[3];
//			};
//		protected:
//			VectorStorageN(){std::memset(this,0,size);;}
//		};
//		template<typename T>
//		struct VectorStorageN<T,4>{
//			static const std::size_t size = 4;
//			typedef typename ParameterType<T>::type parameter_type;
//			VectorStorageN(parameter_type x_,parameter_type y_,parameter_type z_,parameter_type w_)
//				:x(x_),y(y_),z(z_),w(w_){}
//
//			union{
//				struct {T x,y,z,w;};
//				T val[4];
//			};
//
//			protected:
//				VectorStorageN(){std::memset(this,0,size);}
//		};
//		typedef Vector<int,VectorStorageN<int,2> > vector2i;
//		typedef Vector<int,VectorStorageN<int,3> > vector3i;
//		typedef Vector<int,VectorStorageN<int,4> > vector4i;
//
//		typedef Vector<float,VectorStorageN<float,2> > vector2f;
//		typedef Vector<float,VectorStorageN<float,3> > vector3f;
//		typedef Vector<float,VectorStorageN<float,4> > vector4f;
//	}
//}
#ifdef _MSC_VER
#pragma   warning(push) 
#pragma warning( disable : 4181 4244 4127 4211)
#endif

#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/QR>
#include <Eigen/Geometry>
#include <Eigen/Array>

#ifdef _MSC_VER
#pragma   warning(pop) 
#endif 

namespace ma{
		//using eigen lib's vectors

		typedef Eigen::Vector2i vector2i;
		typedef Eigen::Vector3i vector3i;
		typedef Eigen::Vector4i vector4i;
		
		typedef Eigen::Vector2f vector2f;
		typedef Eigen::Vector3f vector3f;
		typedef Eigen::Vector4f vector4f;
		
		//matrices
		typedef Eigen::Matrix2i matrix22i;
		typedef Eigen::Matrix3i matrix33i;
		typedef Eigen::Matrix4i matrix44i;

		typedef Eigen::Matrix2f matrix22f;
		typedef Eigen::Matrix3f matrix33f;
		typedef Eigen::Matrix4f matrix44f;

		typedef vector2i scalar2i;
		
		namespace scalar2_op{
			inline int& width( scalar2i& x){return x[0];}
			inline int& height( scalar2i& x){return x[1];}
			inline int width(const scalar2i& x){return x[0];}
			inline int height(const scalar2i& x){return x[1];}
		}
		namespace vector_op{
			template<typename T,int _Rows, int _Cols, int _StorageOrder, int _MaxRows, int _MaxCols> 
			inline T& x(
				Eigen::Matrix<T,_Rows,_Cols,_StorageOrder,_MaxRows,_MaxCols>& v)
			{
				BOOST_STATIC_ASSERT(_Rows == 1 && _Cols >= 1);
				return v[0];
			}
			template<typename T,int _Rows, int _Cols, int _StorageOrder, int _MaxRows, int _MaxCols> 
			inline T& y(
				Eigen::Matrix<T,_Rows,_Cols,_StorageOrder,_MaxRows,_MaxCols>& v)
			{
				BOOST_STATIC_ASSERT(_Rows == 1 && _Cols >= 2);
				return v[1];
			}
			template<typename T,int _Rows, int _Cols, int _StorageOrder, int _MaxRows, int _MaxCols> 
			inline T& z(
				Eigen::Matrix<T,_Rows,_Cols,_StorageOrder,_MaxRows,_MaxCols>& v)
			{
				BOOST_STATIC_ASSERT(_Rows == 1 && _Cols >= 3);
				return v[2];
			}
			template<typename T,int _Rows, int _Cols, int _StorageOrder, int _MaxRows, int _MaxCols> 
			inline T& w(
				Eigen::Matrix<T,_Rows,_Cols,_StorageOrder,_MaxRows,_MaxCols>& v)
			{
				BOOST_STATIC_ASSERT(_Rows == 1 && _Cols >= 4);
				return v[3];
			}

			//const
			template<typename T,int _Rows, int _Cols, int _StorageOrder, int _MaxRows, int _MaxCols> 
			inline const T& x(
				const Eigen::Matrix<T,_Rows,_Cols,_StorageOrder,_MaxRows,_MaxCols>& v)
			{
				BOOST_STATIC_ASSERT(_Rows == 1 && _Cols >= 1);
				return v[0];
			}
			template<typename T,int _Rows, int _Cols, int _StorageOrder, int _MaxRows, int _MaxCols> 
			inline const T& y(
				const Eigen::Matrix<T,_Rows,_Cols,_StorageOrder,_MaxRows,_MaxCols>& v)
			{
				BOOST_STATIC_ASSERT(_Rows == 1 && _Cols >= 2);
				return v[1];
			}
			template<typename T,int _Rows, int _Cols, int _StorageOrder, int _MaxRows, int _MaxCols> 
			inline const T& z(
				const Eigen::Matrix<T,_Rows,_Cols,_StorageOrder,_MaxRows,_MaxCols>& v)
			{
				BOOST_STATIC_ASSERT(_Rows == 1 && _Cols >= 3);
				return v[2];
			}
			template<typename T,int _Rows, int _Cols, int _StorageOrder, int _MaxRows, int _MaxCols> 
			inline const T& w(
				const Eigen::Matrix<T,_Rows,_Cols,_StorageOrder,_MaxRows,_MaxCols>& v)
			{
				BOOST_STATIC_ASSERT(_Rows == 1 && _Cols >= 4);
				return v[3];
			}

		}
		template<typename _Scalar, int _Rows, int _Cols, int _StorageOrder, int _MaxRows, int _MaxCols>
		inline void swap(Eigen::Matrix<_Scalar,_Rows,_Cols,_StorageOrder,_MaxRows,_MaxCols>& lhs,Eigen::Matrix<_Scalar,_Rows,_Cols,_StorageOrder,_MaxRows,_MaxCols>& rhs)
		{
			lhs.swap(rhs);
		}
}
#endif