#ifndef _MA_INCLUDED_TEXTURE_HPP_
#define _MA_INCLUDED_TEXTURE_HPP_
#include "CRTPInterfaceMacro.hpp"
#include "ptr_var.hpp"
namespace ma{
	//
	namespace texture{
		DECL_FUNC_NEST(scalar_t,mapping,7)
			
		DECL_FUNC_NEST(scalar_t,evaluate,1)
	}

	BEGIN_CRTP_INTERFACE(TextureMapping2D)
	ADD_CRTP_INTERFACE_TYPEDEF(differential_geometry_t)
	ADD_CRTP_INTERFACE_TYPEDEF(scalar_t)

	CRTP_CONST_VOID_METHOD(mapping,7,
	( I_(const differential_geometry_t&,dg),
	 I_(scalar_t&,s), I_(scalar_t&,t), I_(scalar_t&,dsdx), I_(scalar_t&,dtdx),
	 I_(scalar_t&,dsdy), I_(scalar_t&,dtdy)
	))
	END_CRTP_INTERFACE

	///
	//texturea mapping2d
	template<typename Conf>
	class UVMapping2D:public TextureMapping2D<TextureMapping2D<Conf>,typename Conf::interface_config>{
	public:
		ADD_SAME_TYPEDEF(Conf,scalar_t);
		ADD_SAME_TYPEDEF(Conf,differential_geometry_t);
		UVMapping2D(scalar_t su=1,scalar_t sv =1,
			scalar_t du = 0,scalar_t dv = 0)
			:su_(su),sv_(sv),du_(du),dv_(dv){}
		void mappingImpl(const differential_geometry_t& dg,scalar_t& s,scalar_t& t,scalar_t& dsdx,scalar_t& dtdx,
			scalar_t& dsdy,scalar_t& dtdy)
		{
			s = su_ * dg.u + du_;
			t = sv_ * dg.v + dv_;
			dsdx = su_ * dg.dudx;
			dtdx = sv_ * dg.dvdx;
			dsdy = su_ * dg.dudy;
			dtdy = sv_ * dg.dvdy;
		}
	private:
		scalar_t su_,sv_,du_,dv_;
	};
template<typename Conf>
	class SphericalMapping2D:public TextureMapping2D<SphericalMapping2D<Conf>,typename Conf::interface_config>
{
	public:
		ADD_SAME_TYPEDEF(Conf,scalar_t);
		ADD_SAME_TYPEDEF(Conf,differential_geometry_t);
		ADD_SAME_TYPEDEF(Conf,transform_t);
		SphericalMapping2D(const transform_t& t):world2texture_(t){}
		void mappingImpl(const differential_geometry_t& dg,scalar_t& s,scalar_t& t,scalar_t& dsdx,scalar_t& dtdx,
			scalar_t& dsdy,scalar_t& dtdy);
	private:
	transform_t world2texture_;	
};
template<typename Conf>
	class CylindricalMapping2D:public TextureMapping2D<CylindricalMapping2D<Conf>,typename Conf::interface_config>
{
	public:
		ADD_SAME_TYPEDEF(Conf,scalar_t);
		ADD_SAME_TYPEDEF(Conf,differential_geometry_t);
		ADD_SAME_TYPEDEF(Conf,transform_t);
		CylindricalMapping2D(const transform_t& t):world2texture_(t){}
		void mappingImpl(const differential_geometry_t& dg,scalar_t& s,scalar_t& t,scalar_t& dsdx,scalar_t& dtdx,
			scalar_t& dsdy,scalar_t& dtdy);
	private:
	transform_t world2texture_;	
};
template<typename Conf>
	class PlanarMapping2D:public TextureMapping2D<PlanarMapping2D<Conf>,typename Conf::interface_config>
{
	public:
		ADD_SAME_TYPEDEF(Conf,vector_t);
		ADD_SAME_TYPEDEF(Conf,scalar_t);
		ADD_SAME_TYPEDEF(Conf,differential_geometry_t);
		ADD_SAME_TYPEDEF(Conf,transform_t);
		PlanarMapping2D(vector_t& v1,vector_t v2,scalar_t du,scalar_t dv)
		:vs_(v1),vt_(v2),du_(du),dv_(dv){}
		void mappingImpl(const differential_geometry_t& dg,scalar_t& s,scalar_t& t,scalar_t& dsdx,scalar_t& dtdx,
			scalar_t& dsdy,scalar_t& dtdy);
	private:
	vector_t vs_,vt_;
	scalar_t du_,dv_;
};

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
	public:
	ADD_CRTP_INTERFACE_TYPEDEF(scalar_t)
	ADD_CRTP_INTERFACE_TYPEDEF(differential_geometry_t)
	public:
	CRTP_CONST_METHOD(scalar_t,evaluate,1,( I_(const differential_geometry_t&,dg)))
	END_CRTP_INTERFACE

	// ConstantTexture Declarations
	template <class Conf>
	class ConstantTexture : public Texture<ConstantTexture<Conf>,Conf> {
		public:
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
#include "TypeMap.hpp"
namespace ma{
namespace details
{
	template<typename T>
		struct texture_creator;
}
	template<typename T,typename XF,typename TP>
	T* create_texture(const XF& xform,const TP& param)
	{
		return details::texture_creator<T>()(xform,param);
	}


MAKE_TYPE_STR_MAP(1,ConstantTexture,constant)

namespace details
{
	template<typename C>
		struct texture_creator<ConstantTexture<C> >
		{
			typedef ConstantTexture<C> texture_t;
			typedef typename texture_t::scalar_t scalar_t;
			template<typename XF,typename TP>
			texture_t* operator()(const XF& xform,const TP& tp)const
			{
				return new texture_t(tp.template as<scalar_t>("value",scalar_t(1)));	
			}
		};
}
}
#endif
