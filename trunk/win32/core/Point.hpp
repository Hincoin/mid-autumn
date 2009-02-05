#ifndef _MA_INCLUDED_POINT_HPP_
#define _MA_INCLUDED_POINT_HPP_

#include "VectorType.hpp"

namespace ma{
template<typename T> struct scalar_type;
//vector type has both scalar type and dimension
template<typename VectorType>
struct Point{
	typedef typename scalar_type<VectorType>::type scalar_type;
	typedef VectorType value_type;

    Point(){}
    Point(scalar_type x,scalar_type y){ p[0]=x;p[1]=y;}
	Point(scalar_type x,scalar_type y,scalar_type z){ p[0]=x;p[1]=y;p[2]=z;}
    Point(const Point& other):p(other.p){}
    explicit Point(const value_type& v):p(v){}

    void swap(Point& other){p.swap(other.p);}

    Point operator+(const value_type& lhs)const{return Point(p+lhs);}
    Point& operator+=(const value_type& lhs){p+=lhs;return *this;}
    Point operator-(const value_type& lhs)const{return Point(p-lhs);}
    Point& operator-=(const value_type& lhs){p-=lhs;return *this;}
    value_type operator-(const Point& lhs)const{return p-lhs.p;}
	scalar_type operator[](int i)const{return p[i];}
	scalar_type& operator[](int i){return p[i];}
//	private:
	value_type p;
};
template<typename V>
struct dimensions<Point<V> >{
    static const int value = dimensions<V>::value;
    };
    template<typename V>
    struct scalar_type<Point<V> >{
typedef typename scalar_type<V>::type type;
        };

        template<typename V>
        bool equal(const Point<V>& lhs,const Point<V>& rhs)
        {
            return equal(lhs.p,rhs.p);
        }
template<typename P>
inline typename P::scalar_type distance(const P& p1,const P& p2)
{
    return (p1-p2).norm();
}
template<typename P>
inline typename P::scalar_type distanceSquared(const P& p1,const P& p2)
{
    return (p1-p2).squaredNorm();
}

template<typename V>
Point<V> operator*(typename transform_type<typename scalar_type<V>::type,dimensions<V>::value>::type& trans,
				   const Point<V>& v)
{
	return Point<V>(trans * v.p);
}
}
#endif
