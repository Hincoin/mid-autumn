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

#ifndef SPACESEGMENT_HPP
#define SPACESEGMENT_HPP

#include "ParameterType.hpp"

#include "Move.hpp"
#include <boost/operators.hpp>

#include "MAMath.hpp"
#include "Point.hpp"
#include "Vector.hpp"
namespace ma{
	//this class define an "segment" in space (1d,2d,3d)
	template<typename T,class EqualCompare> struct SpaceSegment;

	template<typename T,class EqualCompare>
	inline void swap(SpaceSegment<T, EqualCompare>& a,SpaceSegment<T,EqualCompare>& b);

	namespace details{
		template<typename T> struct default_equal: std::binary_function<T,T,bool>
		{
			bool operator()(const T& lhs,const T& rhs)const
			{
				return equal(lhs,rhs);
			}
		};
	}
	template<typename T, class equality =  details::default_equal<T> >
	struct SpaceSegment:boost::equality_comparable<SpaceSegment<T> >{
	private:
		typedef typename  ParameterType<T>::param_type parameter_type;
	public:
		typedef T stored_type;
		typedef typename scalar_type<T>::type sclr_t;
		typedef Point<T> point_t;

		point_t smin,smax;

		//regular model
		SpaceSegment(){
			for (int i = 0;i < dimensions<point_t>::value; ++i)
			{
				smin[i] = std::numeric_limits<sclr_t>::infinity();
				smax[i] = -std::numeric_limits<sclr_t>::infinity();
			}
		}
		SpaceSegment(parameter_type min_value,parameter_type max_value):smin(min_value),smax(max_value){}
		SpaceSegment(const SpaceSegment& other):smin(other.smin),smax(other.smax)
		{}
		bool operator ==(const SpaceSegment<T,equality>& rhs)const{
			return equality()(smin,rhs.smin) && equal_fun(smax,rhs.smax);//lhs.smin == rhs.smin && lhs.smax == rhs.smax;
		}

		//model concept movable

		SpaceSegment(ma::move_from<SpaceSegment<T,equality> > other):
		smin(ma::move(other.source.smin)),smax(ma::move(other.source.smax)){}
		SpaceSegment& operator=(SpaceSegment other){
			swap(*this,other);return *this;
		}
		SpaceSegment& operator=(ma::move_from<SpaceSegment<T,equality> > other)
		{
			swap(smin,other.source.smin);
			swap(smax,other.source.smax);
			return *this;
		}

		bool isOverlap(const SpaceSegment& other)const{

			typedef SpaceSegment<T> SST;
			typedef typename dimensions<SST>::value_type i_t;
			bool ret = true;
			for (i_t i = 0;i < dimensions<SST>::value; ++i)
			{
				ret = ret && (smin[i] <= other.smax[i] ) && (smax[i] >= other.smin[i]);
			}
			return ret;
		}
		bool isInside(const stored_type& p)const{
			typedef SpaceSegment<T> SST;
			typedef typename dimensions<SST>::value_type i_t;
			bool ret = true;
			for (i_t i = 0;i < dimensions<SST>::value; ++i)
			{
				ret = ret && p[i] >= smin[i] && p[i] <= smax[i];
			}
			return ret;
		}

	};
	template<typename T,class EqualCompare>
	inline void swap(SpaceSegment<T,EqualCompare>& a,SpaceSegment<T,EqualCompare>& b)
	{
	    a.smax.swap(b.smax);
	    a.smin.swap(b.smin);
		//swap(a.smax,b.smax);
		//swap(a.smin,b.smin);
	}
}
#include "ScalarType.hpp"
#include "VectorType.hpp"

namespace ma{
	template<typename T>
	struct dimensions<SpaceSegment<T> >
	{
		typedef int value_type;
		static const int value = dimensions<T>::value;
	};
	template<typename T>
	struct scalar_type<SpaceSegment<T> >
	{
		typedef typename scalar_type<T>::type type;
	};


	template<typename T>
	inline SpaceSegment<T> space_union(const SpaceSegment<T>& s0,const SpaceSegment<T>& s1)
	{
		typedef SpaceSegment<T> SST;
		typedef typename dimensions<SST>::value_type i_t;
		SST ret;
		for (i_t i = 0;i < dimensions<SST>::value; ++i)
		{
			ret.smin[i] = std::min(s0.smin[i],s1.smin[i]);
			ret.smax[i] = std::max(s0.smax[i],s1.smax[i]);
		}
		return ret;
	}

	template<typename T>
	inline SpaceSegment<T> space_union(const SpaceSegment<T>& s0,const Point<T>& p)
	{
		typedef SpaceSegment<T> SST;
		typedef typename dimensions<SST>::value_type i_t;
		SST ret;
		for (i_t i = 0;i < dimensions<SST>::value; ++i)
		{
			ret.smin[i] = std::min(s0.smin[i],p[i]);
			ret.smax[i] = std::max(s0.smax[i],p[i]);
		}
		return ret;
	}

	template<typename T>
	inline SpaceSegment<T> space_union(const Point<T>& p0,const Point<T>& p1)
	{
        typedef SpaceSegment<T> SST;
		typedef typename dimensions<SST>::value_type i_t;
		SST ret;
		for (i_t i = 0;i < dimensions<SST>::value; ++i)
		{
			ret.smin[i] = std::min(ret.smin[i],p0[i]);
			ret.smax[i] = std::max(ret.smax[i],p0[i]);


			ret.smin[i] = std::min(ret.smin[i],p1[i]);
			ret.smax[i] = std::max(ret.smax[i],p1[i]);
		}
		return ret;
    }

    	/// transformation
	template<typename T>
	SpaceSegment<T> operator*(
		const typename transform_type<typename scalar_type<SpaceSegment<T> >::type,dimensions<SpaceSegment<T> >::value >::type& trans,
		const SpaceSegment<T>& seg)
	{
	    ///assert(false);///transform box is not correctly implemented
	    typedef T VectorType;
        typename SpaceSegment<T>::point_type p(trans * seg.smin);
        VectorType v[dimensions<VectorType>::value];

        for(int i = 0;i < dimensions<VectorType>::value;++i)
        {
            v[i][i] = seg.smax[i] - seg.smin[i];
            v[i].swap( trans * v[i] );
        }
        SpaceSegment<T> ret;
        ret.swap(space_union(ret,p));
        for (int i = 0;i < dimensions<VectorType>::value; ++i)
        {ret.swap(space_union(ret,p+v[i]));}
        for(int i = 0;i < dimensions<VectorType>::value;++i)
        {
            p += v[i];
        }
        ret.swap(space_union(ret,p));
        for (int i = 0;i < dimensions<VectorType>::value; ++i)
        {ret.swap(space_union(ret,p-v[i]));}

		return ret;
	}
}

#include "Vector.hpp"
namespace ma{

	//make vectors to be movable
	//Matrix<Type, Size, 1>

	//template<typename T,int Size>
	//struct is_movable< Eigen::Matrix<T,Size , 1> >
	//	:boost::mpl::true_
	//{};

	//1d
	typedef SpaceSegment<int> rangei;
	typedef SpaceSegment<float> rangef;

	//2d
	typedef SpaceSegment<vector2i> recti;//0 left 1 top
	typedef SpaceSegment<vector2f> rectf;

	namespace rect_op{
		template<typename T>
		inline
		static typename T::stored_type::Scalar& left(T& x){return x.smin[0];}

		template<typename T>
		inline
		static typename T::stored_type::Scalar& top(T& x){return x.smin[1];}

		template<typename T>
		inline
		static typename T::stored_type::Scalar& right(T& x){return x.smax[0];}

		template<typename T>
		inline
		static typename T::stored_type::Scalar& bottom(T& x){return x.smax[1];}

		template<typename T>
		inline
		static typename T::stored_type::Scalar width(const T& x){return x.smax[0]-x.smin[0];}

		template<typename T>
		inline
		static typename T::stored_type::Scalar height(const T& x){return x.smax[1] - x.smin[1];}
	}

	//3d
	typedef SpaceSegment<vector3i> boxi;
	typedef SpaceSegment<vector3f> boxf;

}

#endif
