#ifndef _MA_INCLUDED_POINT_HPP_
#define _MA_INCLUDED_POINT_HPP_

#include "VectorType.hpp"

namespace ma{
template<typename T> struct scalar_type;
//vector type has both scalar type and dimension
template<typename VectorType>
struct Point{
	typedef typename scalar_type<VectorType>::type scalar_t;
	typedef VectorType value_type;

    Point(){}
    Point(scalar_t x,scalar_t y){ p[0]=x;p[1]=y;}
	Point(scalar_t x,scalar_t y,scalar_t z){ p[0]=x;p[1]=y;p[2]=z;}
    Point(const Point& other):p(other.p){}
    explicit Point(const value_type& v):p(v){}

    void swap(Point& other){p.swap(other.p);}

    Point operator+(const value_type& lhs)const{return Point(p+lhs);}
    Point& operator+=(const value_type& lhs){p+=lhs;return *this;}
    Point operator-(const value_type& lhs)const{return Point(p-lhs);}
    Point& operator-=(const value_type& lhs){p-=lhs;return *this;}
    value_type operator-(const Point& lhs)const{return p-lhs.p;}
	scalar_t operator[](int i)const{return p[i];}
	scalar_t& operator[](int i){return p[i];}

	scalar_t& x(){return p.x();}
	const scalar_t x()const{return p.x();}

	scalar_t& y(){return p.y();}
	const scalar_t y()const{return p.y();}

	scalar_t& z(){return p.z();}
	const scalar_t z()const{return p.z();}
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
inline typename P::scalar_t distance(const P& p1,const P& p2)
{
    return (p1-p2).norm();
}
template<typename P>
inline typename P::scalar_t distanceSquared(const P& p1,const P& p2)
{
    return (p1-p2).squaredNorm();
}

template <typename T=float,int D=3>
struct point_type{
	typedef Point<typename vector_type<T,D>::type > type;
};

}
#endif
