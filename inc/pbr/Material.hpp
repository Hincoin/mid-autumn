#ifndef _MA_INCLUDED_MATERIAL_HPP_
#define _MA_INCLUDED_MATERIAL_HPP_
#include "CRTPInterfaceMacro.hpp"

namespace ma{
	BEGIN_CRTP_INTERFACE(Material)
		ADD_CRTP_INTERFACE_TYPEDEF(bsdf_ptr)
		ADD_CRTP_INTERFACE_TYPEDEF(differential_geometry_t)
		ADD_CRTP_INTERFACE_TYPEDEF(texture_scalar_t_ref)
public:
	ADD_CRTP_INTERFACE_TYPEDEF(transform_t)
public:
	CRTP_CONST_METHOD(bsdf_ptr,getBSDF,2,
		(IN(const differential_geometry_t&,dgGeom),IN(const differential_geometry_t&,dgShading)));
	static void bump(texture_scalar_t_ref d,const differential_geometry_t& dgGeom,
		const differential_geometry_t& dgShading,differential_geometry_t& dgBump)
	{
		//// Compute offset positions and evaluate displacement texture
		//DifferentialGeometry dgEval = dgs;
		//// Shift _dgEval_ _du_ in the $u$ direction
		//float du = .5f * (fabsf(dgs.dudx) + fabsf(dgs.dudy));
		//if (du == 0.f) du = .01f;
		//dgEval.p = dgs.p + du * dgs.dpdu;
		//dgEval.u = dgs.u + du;
		//dgEval.nn =
		//	Normalize((Normal)Cross(dgs.dpdu, dgs.dpdv) +
		//	du * dgs.dndu);
		//float uDisplace = d->Evaluate(dgEval);
		//// Shift _dgEval_ _dv_ in the $v$ direction
		//float dv = .5f * (fabsf(dgs.dvdx) + fabsf(dgs.dvdy));
		//if (dv == 0.f) dv = .01f;
		//dgEval.p = dgs.p + dv * dgs.dpdv;
		//dgEval.u = dgs.u;
		//dgEval.v = dgs.v + dv;
		//dgEval.nn =
		//	Normalize((Normal)Cross(dgs.dpdu, dgs.dpdv) +
		//	dv * dgs.dndv);
		//float vDisplace = d->Evaluate(dgEval);
		//float displace = d->Evaluate(dgs);
		//// Compute bump-mapped differential geometry
		//*dgBump = dgs;
		//dgBump->dpdu = dgs.dpdu +
		//	(uDisplace - displace) / du * Vector(dgs.nn) +
		//	displace * Vector(dgs.dndu);
		//dgBump->dpdv = dgs.dpdv +
		//	(vDisplace - displace) / dv * Vector(dgs.nn) +
		//	displace * Vector(dgs.dndv);
		//dgBump->nn =
		//	Normal(Normalize(Cross(dgBump->dpdu, dgBump->dpdv)));
		//if (dgs.shape->reverseOrientation ^
		//	dgs.shape->transformSwapsHandedness)
		//	dgBump->nn *= -1.f;
		//// Orient shading normal to match geometric normal
		//if (Dot(dgGeom.nn, dgBump->nn) < 0.f)
		//	dgBump->nn *= -1.f;
	}
	END_CRTP_INTERFACE
}

#include "ParamSet.hpp"

namespace ma{
	namespace details{
		template<typename M>
		struct material_creator;
	}
	template<typename M>
	M* create_material(const typename M::transform_t& xform,const ParamSet& mp)
	{
		return details::material_creator<M>()(xform,mp);
	}
}
#endif