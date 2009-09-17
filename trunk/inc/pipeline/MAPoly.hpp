#ifndef MA_TRIANGLE_HPP
#define MA_TRIANGLE_HPP

#include "Vector.hpp"
#include "Move.hpp"

#include <boost/static_assert.hpp>
#include <boost/mpl/bool.hpp>
#include "MAVertex.hpp"

#include <cassert>
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
	void compute_normal_impl(const vertex_type& v0,const vertex_type& v1,const vertex_type& v2,normal_type& n,
		boost::mpl::bool_<true>* ,boost::mpl::bool_<true>*)const
	{
		n.swap((v2.position()-v0.position()).cross(v1.position() - v0.position()));
	}
	void compute_normal_impl(const vertex_type& v0,const vertex_type& v1,const vertex_type& v2,normal_type& n,
		boost::mpl::bool_<false>* ,boost::mpl::bool_<true>* )const
	{
		n.swap((v1.position()-v0.position()).cross(v2.position()-v0.position()));
	}
	template<bool t_f>
	void compute_normal_impl(const vertex_type& v0,const vertex_type& v1,const vertex_type& v2,normal_type& n,
		boost::mpl::bool_<t_f>*  ,boost::mpl::bool_<false>* ) const
	{}

	void compute_normal(const vertex_type& v0,const vertex_type& v1,const vertex_type& v2,normal_type& n)const
	{
		compute_normal_impl(v0,v1,v2,static_cast<normal_type&>(*this),
			(boost::mpl::bool_<direction == MA_ANTI_CLOCK_WISE>*) 0,
			(boost::mpl::bool_<normal_dimension == 3>*) 0);
	}
	void recompute_normal(const vertex_type& v0,const vertex_type& v1,const vertex_type& v2,normal_type& n)const
	{
		compute_normal_impl(v0,v1,v2,n,
			(boost::mpl::bool_<direction == MA_ANTI_CLOCK_WISE>*) 0,
			(boost::mpl::bool_<N == 3>*)(0));
	}

	void swap_normal_impl(normal_type& n0,normal_type& n1, boost::mpl::bool_<true>*)const
	{n0.swap(n1);}
	void swap_normal_impl(normal_type& n0,normal_type& n1, boost::mpl::bool_<false>*)const
	{}
	void swap_normal(normal_type& n0,normal_type& n1)const
	{
		swap_normal_impl(n0,n1,(boost::mpl::bool_<normal_dimension == 3>*) 0);
	}
public:
	//default ctor
	MAPolygonFixedSize(){}

	MAPolygonFixedSize(const vertex_type& v0,const vertex_type& v1,const vertex_type& v2){
		verts[0]=v0;
		verts[1]=v1;
		verts[2]=v2;
		compute_normal(v0,v1,v2,*this);
	}
	MAPolygonFixedSize(const MAPolygonFixedSize& poly):
	normal_type(static_cast<const normal_type&>(poly)){
		for (int i = 0;i < N;++i)
		{
			verts[i] = poly.verts[i];
		}
	}
	template<typename Normal_Type>
	MAPolygonFixedSize(const MAPolygonFixedSize<vertex_type,Normal_Type,N,direction>& other){
		for (int i = 0;i < N;++i)
		{
			verts[i] = other.verts[i];
		}
		compute_normal( verts[0] ,verts[1] ,verts[1],*this);
	}

	//swappable
	void swap(MAPolygonFixedSize& other)
	{
		vertex_type* t = other.verts;
		verts = other.verts;
		other.verts = t;
		swap_normal(*this,other);
	}
	MAPolygonFixedSize& operator = (MAPolygonFixedSize other){
		swap(other);
		return *this;
	}
	//modeling move semantics
	MAPolygonFixedSize(ma::move_from<MAPolygonFixedSize> poly)
	{
		for (int i = 0;i < N; ++i)//maybe unrolled by a good compiler
		{
			verts[i] = ma::move(poly.source.verts[i]);
		}
		swap_normal(*this,poly.source);
	}
	MAPolygonFixedSize& operator = (ma::move_from<MAPolygonFixedSize> poly){
		swap(poly.source);
		return *this;
	}

	//common operations
	template<int Idx>
	void setVertex(const vertex_type& v){
		BOOST_STATIC_ASSERT(Idx < N);
		verts[Idx] = v;
		recompute_normal(v[0],v[1],v[2],*this);
	}
	template<int Idx>
	const vertex_type& getVertex()const{BOOST_STATIC_ASSERT(Idx < N); return verts[Idx];}

	//runtime getter setter
	void setVertex(size_t idx,const vertex_type& v){
		assert(((idx < N) && "index out of bound"));
		verts[idx] = v;
		recompute_normal(v[0],v[1],v[2],*this);
	}
	const vertex_type& getVertex(size_t idx)const{
		assert(((idx < N) && "index out of bound"));
		return verts[idx];
	}

	const normal_type& normal()const{return static_cast<const normal_type&>(*this);}
	const normal_type& recompute_normal()const{recompute_normal(verts[0], verts[1], verts[2],*this);return *this;}
private:
	vertex_type verts[N];
};

template<typename Vertex_t,typename Normal_T,unsigned int N  ,MA_POLYGON_DIR dir>
struct dimensions<MAPolygonFixedSize<Vertex_t,Normal_T,N,dir> >{enum{value = N};};

namespace ma_traits{
	template<typename Polygon_Type>
	struct polygon_traits;

	template<typename Vertex_t,typename Normal_T,unsigned int N ,MA_POLYGON_DIR dir>
	struct polygon_traits<MAPolygonFixedSize<Vertex_t,Normal_T,N,dir> >{

		typedef Vertex_t vertex_type;
		static const int polygon_dimension = dimensions<vertex_type>::value;
		static const int vertex_count = N;
		static const bool is_fixed = true; // indicate the vertex_count of every face is fixed or variable
		enum{direction = dir};

	};
}
namespace triangle_op{
	//normal

	//
}
}
#endif
