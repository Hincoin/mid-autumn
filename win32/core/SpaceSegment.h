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

#ifndef SPACESEGMENT_H
#define SPACESEGMENT_H

#include "ParameterType.h"

#include "Move.h"


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
				return lhs == rhs;
			}
		};
	}
	template<typename T, class equality =  details::default_equal<T> >
	struct SpaceSegment:boost::equality_comparable<SpaceSegment<T> >{
	private:
		typedef typename  ParameterType<T>::param_type parameter_type;
	public:
		T smin,smax;

		//regular model
		SpaceSegment(){}
		SpaceSegment(parameter_type min_value,parameter_type max_value):smin(min_value),smax(max_value){}
		SpaceSegment(const SpaceSegment& other):smin(other.smin),smax(other.smax)
		{}
		friend bool operator ==(const SpaceSegment<T,equality>& lhs,const SpaceSegment<T,equality>& rhs){
			equality equal_fun;
			return equal_fun(lhs.smin,rhs.smin) && equal_fun(lhs.smax,rhs.smax);//lhs.smin == rhs.smin && lhs.smax == rhs.smax;
		}

		//model concept movable

		SpaceSegment(ma::move_from<SpaceSegment<T,equality> > other):
		smin(ma::move(other.source.smin)),smax(ma::move(other.source.smax)){}
		SpaceSegment& operator=(SpaceSegment other){
			swap(*this,other);return *this;
		}
	};
	template<typename T,class EqualCompare>
	inline void swap(SpaceSegment<T,EqualCompare>& a,SpaceSegment<T,EqualCompare>& b)
	{
		a.smin.swap(b.smin);
		a.smax.swap(b.smax);
	}
}


#include "Vector.h"
namespace ma{

	//make vectors to be movable
	//Matrix<Type, Size, 1>

	template<typename T,int Size>
	struct is_movable< Eigen::Matrix<T,Size , 1> >
		:boost::mpl::true_
	{};

	//1d
	typedef SpaceSegment<int> rangei;
	typedef SpaceSegment<float> rangef;

	//2d
	typedef SpaceSegment<vector2i> recti;
	typedef SpaceSegment<vector2f> rectf;

	//3d
	typedef SpaceSegment<vector3i> boxi;
	typedef SpaceSegment<vector3f> boxf;

}

#endif