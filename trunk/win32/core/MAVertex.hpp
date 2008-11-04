#ifndef MA_VERTEX_HPP
#define MA_VERTEX_HPP

#include "Vector.hpp"
#include "NullType.hpp"

namespace ma{
	//a very simple  vertex type only have position information
	template<typename Vector_t,typename Normal_t = EmptyType,typename Color_t  = EmptyType,typename TexCoords_t  = EmptyType>
	struct MAVertex:Vector_t,Normal_t,Color_t,TexCoords_t{
	
		typedef typename scalar_type<Vector_t>::type position_scalar_type;
		typedef typename scalar_type<TexCoords_t>::type texture_coord_scalar_type;
		typedef Vector_t position_type;
		typedef Normal_t normal_type;
		typedef Color_t color_type;
		typedef TexCoords_t texture_coord_type;

		position_type& position(){static_cast<position_type&>(*this);}	
		const position_type& position()const{static_cast<const position_type&>(*this);}
		
		position_type& x(){return vector_op::x(position());}
		position_type& y(){return vector_op::y(position());}
		position_type& z(){return vector_op::z(position());}

		const position_type& x()const{return vector_op::x(position());}
		const position_type& y()const{return vector_op::y(position());}
		const position_type& z()const{return vector_op::z(position());}

		texture_coord_type& texture_coord(){static_cast<texture_coord_type&>(*this);}
		const texture_coord_type& texture_coord()const {static_cast<const texture_coord_type&>(*this);}

		
		texture_coord_scalar_type& u(){return vector_op::u(texture_coord());}
		texture_coord_scalar_type& v(){return vector_op::v(texture_coord());}
		texture_coord_scalar_type& w(){return vector_op::w(texture_coord());}

		const texture_coord_scalar_type& u()const{return vector_op::u(texture_coord());}
		const texture_coord_scalar_type& v()const{return vector_op::v(texture_coord());}
		const texture_coord_scalar_type& w()const{return vector_op::w(texture_coord());}

		color_type& color(){static_cast<color_type&>(*this);}
		const color_type& color()const {static_cast<const color_type&>(*this);}

		normal_type& normal(){static_cast<normal_type&>(*this);}
		const normal_type& normal()const{static_cast<const normal_type&>(*this);}
	};
}

#endif