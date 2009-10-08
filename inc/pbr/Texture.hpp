#ifndef _MA_INCLUDED_TEXTURE_HPP_
#define _MA_INCLUDED_TEXTURE_HPP_
#include "CRTPInterfaceMacro.hpp"

namespace ma{
	BEGIN_CRTP_INTERFACE(TextureMapping2D)
	ADD_CRTP_INTERFACE_TYPEDEF(differential_geometry_t)
	ADD_CRTP_INTERFACE_TYPEDEF(scalar_t)

	CRTP_CONST_VOID_METHOD(mapping,7,
	( I_(const differential_geometry_t&,dg),
	 I_(scalar_t&,s), I_(scalar_t&,t), I_(scalar_t&,dsdx), I_(scalar_t&,dtdx),
	 I_(scalar_t&,dsdy), I_(scalar_t&,dtdy)
	))
	END_CRTP_INTERFACE

	//////////////////////////////////////////////////////////////////////////
	BEGIN_CRTP_INTERFACE(TextureMapping3D)
	ADD_CRTP_INTERFACE_TYPEDEF(differential_geometry_t)
	ADD_CRTP_INTERFACE_TYPEDEF(vector_t)

	CRTP_CONST_VOID_METHOD(mapping,3,
	( I_(const differential_geometry_t&,dg),
	 I_(vector_t&,dpdx), I_(vector_t&,dpdy)
	))
		END_CRTP_INTERFACE
	//////////////////////////////////////////////////////////////////////////
	BEGIN_CRTP_INTERFACE(Texture)
	ADD_CRTP_INTERFACE_TYPEDEF(scalar_t)
	ADD_CRTP_INTERFACE_TYPEDEF(differential_geometry_t)
public:
	CRTP_CONST_METHOD(scalar_t,evaluate,1,( I_(const differential_geometry_t&,dg)))
	END_CRTP_INTERFACE

	// ConstantTexture Declarations
	template <class Conf>
	class ConstantTexture : public Texture<ConstantTexture<Conf>,Conf> {
		ADD_SAME_TYPEDEF(Conf,scalar_t)
		ADD_SAME_TYPEDEF(Conf,differential_geometry_t)
	public:
		// ConstantTexture Public Methods
		ConstantTexture(const scalar_t &v) { value = v; }
		scalar_t evaluateImpl(const differential_geometry_t &) const {
			return value;
		}
	private:
		scalar_t value;
	};
}
#endif