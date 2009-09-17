#ifndef MA_VERTEX_HPP
#define MA_VERTEX_HPP

#include "Vector.hpp"
#include "NullType.hpp"

namespace ma{
	//a very simple  vertex type only have position information
	template<typename Vector_t,
		typename Normal_t = MultiEmptyType<0>,
		typename Color_t  = MultiEmptyType<1>,
		typename TexCoords_t  = MultiEmptyType<2> >
	struct MAVertex:
		MultiParentType<Vector_t,0>,
		MultiParentType<Normal_t,1>,
		MultiParentType<Color_t,2>,
		MultiParentType<TexCoords_t,3>{

		typedef	MultiParentType<Vector_t,0> position_base;
		typedef		MultiParentType<Normal_t,1> normal_base;
		typedef		MultiParentType<Color_t,2> color_base;
		typedef		MultiParentType<TexCoords_t,3> texture_coord_base;

		typedef typename scalar_type<Vector_t>::type position_scalar_type;
		static const int position_dimension = dimensions<Vector_t>::value;

		typedef typename scalar_type<TexCoords_t>::type texture_coord_scalar_type;
		static const int texture_coord_dimension = dimensions<texture_coord_scalar_type>::value;

		typedef Vector_t position_type;
		typedef Normal_t normal_type;
		typedef Color_t color_type;
		typedef TexCoords_t texture_coord_type;
	public:
		MAVertex(){}
		MAVertex(const position_type& p):position_base(p){}
		MAVertex(position_scalar_type p0,position_scalar_type p1,position_scalar_type p2):position_base(typename position_base::base_type(p0,p1,p2)){}

		position_type& position(){return static_cast<position_type&>((*this).position_base::toParent());}
		const position_type& position()const{return static_cast<const position_type&>((*this).position_base::toParent());}

		position_scalar_type& x(){return vector_op::x(position());}
		position_scalar_type& y(){return vector_op::y(position());}
		position_scalar_type& z(){return vector_op::z(position());}

		const position_scalar_type& x()const{return vector_op::x(position());}
		const position_scalar_type& y()const{return vector_op::y(position());}
		const position_scalar_type& z()const{return vector_op::z(position());}

		texture_coord_type& texture_coord(){return static_cast<texture_coord_type&>((*this).texture_coord_base::toParent());}
		const texture_coord_type& texture_coord()const {return static_cast<const texture_coord_type&>((*this).texture_coord_base::toParent());}


		texture_coord_scalar_type& u(){return vector_op::u(texture_coord());}
		texture_coord_scalar_type& v(){return vector_op::v(texture_coord());}
		texture_coord_scalar_type& w(){return vector_op::w(texture_coord());}

		const texture_coord_scalar_type& u()const{return vector_op::u(texture_coord());}
		const texture_coord_scalar_type& v()const{return vector_op::v(texture_coord());}
		const texture_coord_scalar_type& w()const{return vector_op::w(texture_coord());}

		color_type& color(){static_cast<color_type&>((*this).color_base::toParent());}
		const color_type& color()const {return static_cast<const color_type&>((*this).color_base::toParent());}

		normal_type& normal(){static_cast<normal_type&>((*this).normal_base::toParent());}
		const normal_type& normal()const{return static_cast<const normal_type&>((*this).normal_base::toParent());}
	};

	template<typename Vector_t,
		typename Normal_t,
		typename Color_t ,
		typename TexCoords_t  >
	struct dimensions<MAVertex<Vector_t,Normal_t,Color_t,TexCoords_t> >{
		enum{value = dimensions<Vector_t>::value };
	};
	namespace ma_traits{
		template<typename Vertex_Type>
		struct vertex_traits;

		template<typename Vector_t,
			typename Normal_t,
			typename Color_t,
			typename TexCoords_t >
		struct vertex_traits<MAVertex< Vector_t,Normal_t, Color_t, TexCoords_t > >
		{};

		template<typename Vertex_Type>
		struct vertex_normal_policy{
			static void calculate(){}
		};
	}
}

#endif
