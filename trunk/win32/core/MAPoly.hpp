#ifndef MA_TRIANGLE_HPP
#define MA_TRIANGLE_HPP

#include "Vector.hpp"
#include "Move.hpp"

#include <boost/static_assert.hpp>
#include <boost/mpl/bool.hpp>
#include "MAVertex.hpp"

namespace ma{

enum MA_POLYGON_DIR{MA_CLOCK_WISE = 0,MA_ANTI_CLOCK_WISE};

//fixed size poly
template<typename Vertex_t,typename Normal_t=EmptyType,unsigned int N = 3,MA_POLYGON_DIR dir = MA_ANTI_CLOCK_WISE>
struct MAPolygonFixedSize:Normal_t{
	typedef Vertex_t vertex_type;
	typedef Normal_t normal_type;

	static const int polygon_dimension = dimensions<vertex_type>::value;
	static const int normal_dimension = dimensions<normal_type>::value;
	static const int vertex_count = N;
	static const int direction = dir;
	
private:
	void compute_normal_impl(vertex_type& v0,vertex_type& v1,vertex_type& v2,normal_type& n,boost::bool_<true>* = 0,boost::bool_<false>* = 0)const
	{
		n = (v2.position()-v0.position()).cross(v1.position() - v0.position());
	}
	void compute_normal_impl(vertex_type& v0,vertex_type& v1,vertex_type& v2,normal_type& n,boost::bool_<false>* = 0,boost::bool_<false>* = 0)const
	{
		n = (v1.position()-v0.position()).cross(v2.position()-v0.position());
	}
	void compute_normal_impl(vertex_type& v0,vertex_type& v1,vertex_type& v2,normal_type& n,boost::bool_<false>*  = 0 ,boost::bool_<true>*  = 0 ) const
	{}
	
	void compute_normal(vertex_type& v0,vertex_type& v1,vertex_type& v2,normal_type& n)const
	{
		compute_normal_impl(v0,v1,v2,static_cast<normal_type&>(*this),
			(boost::bool_<direction == MA_ANTI_CLOCK_WISE>*) 0, 
			(boost::bool_<normal_dimension == 3>*) 0);
	}
public:

	
	//default ctor
	MAPolygonFixedSize(){}
	MAPolygonFixedSize(const vertex_type& v0,const vertex_type& v1,const vertex_type& v2){
		verts[0]=v0;
		verts[1]=v1;
		verts[2]=v2;
		compute_normal(v0,v1,v2);
	}
	MAPolygonFixedSize(const MAPolygonFixedSize& poly){
		for (int i = 0;i < N;++i)
		{
			verts[i] = poly.verts[i];
		}
	}

	void swap(MAPolygonFixedSize& other)
	{
		vertex_type* t = other.verts;
		verts = other.verts;
		other.verts = t;
	}
	MAPolygonFixedSize& operator = (MAPolygonFixedSize other){
		swap(other);
		return *this;
	}
	
	//modeling move semantics
	MAPolygonFixedSize(ma::move_from<MAPolygonFixedSize> poly)
	{
		for (int i = 0;i < N; ++i)
		{
			verts[i] = ma::move(poly.source.verts[i]);
		}
	}
	MAPolygonFixedSize& operator = (ma::move_from<MAPolygonFixedSize> poly){
		swap(poly.source);
		return *this;
	}

	//common operations
	template<int Idx>
	void setVertex(const vertex_type& v){BOOST_STATIC_ASSERT(Idx < N);verts[Idx] = v;}
	template<int Idx>
	const vertex_type& getVertex()const{BOOST_STATIC_ASSERT(Idx < N); return verts[Idx];}

	const normal_type& normal()const{return static_cast<const normal_type&>(*this);}

	vertex_type verts[N];
};

template<typename Vertex_t,unsigned int N  ,MA_POLYGON_DIR dir>
struct dimensions<MAPolygonFixedSize<Vertex_t,N,dir> >{enum{value = N};};

namespace ma_traits{
	template<typename Polygon_Type>
	struct polygon_traits;

	template<typename Vertex_t,unsigned int N ,MA_POLYGON_DIR dir>
	struct polygon_traits<MAPolygonFixedSize<Vertex_t,N,dir> >{

		typedef Vertex_t vertex_type;
		static const int polygon_dimension = dimensions<vertex_type>::value;
		static const int vertex_count = N;
		enum{direction = dir};
	};
}
namespace triangle_op{
	//normal

	//
}
}
#endif