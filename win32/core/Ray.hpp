#ifndef _MA_INCLUDED_RAY_HPP_
#define _MA_INCLUDED_RAY_HPP_
#include <limits>
#include "Point.hpp"

namespace ma{
template<typename V>
struct Ray{
	typedef Point<V> point_type;
	typedef V vector_type;
	typedef scalar_type<V>::type scalar_type;
	static const scalar_type epsilon;

	Ray(const point_type& p,const vector_type& v,
	scalar_type s=epsilon,
	scalar_type e=std::numeric_limits<scalar_type>::max())
	:o(p),dir(v),mint(s),maxt(e){}

    point_type operator(scalar_type t)const{return o+dir*t;}

    vector_type& dir(){return dir;}
    const vector_type& dir()const{return dir;}


	point_type o;
	vector_type dir;
	mutable scalar_type mint,maxt;
};
template< typename V>
const Ray< V>::scalar_type Ray<P,V>::epsilon = 1e-3f;

template<typename V>
Ray<V> operator *(
				  const typename transform_type<typename scalar_type<V>::type,dimensions<V>::value>::type& trans,
				  const Ray<V>& r)
{
	///
	Ray<V> ret;
	ret.o = trans * r.o;
	ret.dir = trans * r.dir;
	ret.mint = r.mint;
	ret.maxt = r.maxt;
	return ret;
}
}
#endif
