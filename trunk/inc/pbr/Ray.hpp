#ifndef _MA_INCLUDED_RAY_HPP_
#define _MA_INCLUDED_RAY_HPP_
#include <limits>
#include "Point.hpp"
#include "CRTPInterfaceMacro.hpp"
#include "serialization.hpp"
namespace ma{


template<typename V>
struct Ray:public serialization::serializable<Ray<V> >{
	typedef Point<V> point_type;
	typedef V vector_type;
	typedef typename scalar_type<V>::type ScalarType;
	static const ScalarType epsilon;
	typedef Ray<V> class_type;
	Ray(){}
	Ray(const point_type& p,const vector_type& v,
	ScalarType s=epsilon,
	ScalarType e=std::numeric_limits<ScalarType>::max())
	:o(p),dir(v),mint(s),maxt(e){}

    point_type operator()(ScalarType t)const{return o+dir*t;}

    vector_type& direction(){return dir;}
    const vector_type& direction()const{return dir;}

	void swap(class_type& other){
		o.swap(other.o);
		dir.swap(other.dir);
		std::swap(mint,other.mint);
		std::swap(maxt,other.maxt);
	}
	public:
		void serializeImpl(std::ostream& out)const
		{
			//todo
		}
		void deserializeImpl(std::istream& in)
		{
			//todo
		}
	

	point_type o;
	vector_type dir;
	mutable ScalarType mint,maxt;
};
template< typename V>
const typename Ray< V>::ScalarType Ray<V>::epsilon = 1e-3f;




template<typename V>
struct RayDifferential:public Ray<V>{
	bool has_differential;
	Ray<V> rx,ry;
	typedef Point<V> point_t;
	typedef V vector_t;
	typedef RayDifferential<V> class_type;
	typedef Ray<V> parent_type;
private:
public:
	class_type& operator=(class_type other)
	{
		swap(other);
		return *this;
	}
	RayDifferential(){has_differential= false;}
	RayDifferential(const point_t& org,const vector_t& dir)
		:Ray<V>(org,dir){has_differential=false;}
	explicit RayDifferential(const Ray<V>& r):Ray<V>(r){
		has_differential = false;
	}
	void swap(class_type& other){
		parent_type::swap(other);
		std::swap(has_differential,other.has_differential);
		rx.swap(other.rx);
		ry.swap(other.ry);
	}
};

template<typename Conf>
struct Intersection{
	ADD_SAME_TYPEDEF(Conf,primitive_ptr)
	ADD_SAME_TYPEDEF(Conf,ray_differential_t);
	ADD_SAME_TYPEDEF(Conf,vector_t)
	ADD_SAME_TYPEDEF(Conf,differential_geometry_t)
	ADD_SAME_TYPEDEF(Conf,transform_t)
	ADD_SAME_TYPEDEF(Conf,spectrum_t)
	ADD_SAME_TYPEDEF(Conf,bsdf_ptr)
public:
	Intersection():primitive(0){}
	bsdf_ptr getBSDF(const ray_differential_t& ray)const{
		dg.computeDifferentials(ray);
		return primitive->getBSDF(dg,world2obj);
	}
	spectrum_t le(const vector_t& wo)const{
		//for area light
		return spectrum_t(0);
	}

	differential_geometry_t dg;
	primitive_ptr primitive;
	transform_t world2obj;
};


template<typename T=float,int D=3>
struct ray_type{
	typedef Ray <typename vector_type<T,D>::type > type;
};
template<typename V>
inline Ray<V> operator *(
						 const typename transform_type<typename scalar_type<V>::type,dimensions<V>::value>::type& trans,
						 const Ray<V>& r)
{
	///
	Ray<V> ret;
	ret.o.p = trans * r.o.p;
	ret.dir = trans.linear() * r.dir;
	ret.mint = r.mint;
	ret.maxt = r.maxt;
	return ret;
}

//namespace details{
//	template<typename S,int D,typename T>
//	struct transformer_impl;
//	template<typename S,int D>
//	struct transformer_impl<S,D,typename ray_type<S,D>::type>
//	{
//		typedef typename ray_type<S,D>::type result_type;
//		typedef result_type param_type;
//		static result_type run(const typename transform_type<S,D>::type& trans,const param_type& r)
//		{
//			result_type ret;
//			ret.o = trans * r.o;
//			ret.dir = trans.linear() * r.dir;
//			ret.mint = r.mint;
//			ret.maxt = r.maxt;
//			return ret;
//		}
//	};
//}

}
#endif
