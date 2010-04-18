#ifndef _MA_INCLUDED_TEXTURE_HPP_
#define _MA_INCLUDED_TEXTURE_HPP_
#include "CRTPInterfaceMacro.hpp"
#include "ptr_var.hpp"
#include "MAMath.hpp"

namespace ma{
	//
	namespace texture{
		DECL_FUNC(void,mapping,7)
			
		DECL_FUNC_NEST(color_t,evaluate,1)
	}

	BEGIN_CRTP_INTERFACE(TextureMapping2D)
	ADD_CRTP_INTERFACE_TYPEDEF(differential_geometry_t)
	ADD_CRTP_INTERFACE_TYPEDEF(scalar_t)

	CRTP_CONST_VOID_METHOD(mapping,7,
	( I_(const differential_geometry_t&, dg),
	 I_(scalar_t&,s), I_(scalar_t&,t), I_(scalar_t&,dsdx), I_(scalar_t&,dtdx),
	 I_(scalar_t&,dsdy), I_(scalar_t&,dtdy)
	))
	END_CRTP_INTERFACE

	///
	//texturea mapping2d
	template<typename Conf>
	class UVMapping2D:public TextureMapping2D<UVMapping2D<Conf>,typename Conf::interface_config>{
	public:
		ADD_SAME_TYPEDEF(Conf,scalar_t);
		ADD_SAME_TYPEDEF(Conf,differential_geometry_t);
		UVMapping2D(scalar_t su=1,scalar_t sv =1,
			scalar_t du = 0,scalar_t dv = 0)
			:su_(su),sv_(sv),du_(du),dv_(dv){}
		void mappingImpl(const differential_geometry_t& dg,scalar_t& s,scalar_t& t,scalar_t& dsdx,scalar_t& dtdx,
			scalar_t& dsdy,scalar_t& dtdy)const
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
			scalar_t& dsdy,scalar_t& dtdy)const;
	private:
		void sphere(const typename Conf::point_t& p,scalar_t& s,
				scalar_t& t)const
		{
			typedef typename Conf::vector_t vector_t;
			typedef typename Conf::point_t point_t;
	vector_t vec = (point_t(world2texture_ * p) - point_t(0,0,0)).normalized();
	scalar_t theta = vector_op::spherical_theta(vec);
	scalar_t phi = vector_op::spherical_phi(vec);
	s = theta * INV_PI;
	t = phi * INV_TWOPI;

		}
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
			scalar_t& dsdy,scalar_t& dtdy)const;
		void cylinder(const typename Conf::point_t& p,scalar_t& s,scalar_t& t)const
		{
			typedef typename Conf::vector_t vector_t;
			typedef typename Conf::point_t point_t;
			vector_t vec = (point_t(world2texture_ * p) - point_t(0,0,0)).normalized();
		s = (M_PI + std::atan2(vec[1],vec[0])) * INV_TWOPI;	
		t = vec[2];
		}
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
		PlanarMapping2D(const vector_t& v1,const vector_t& v2,scalar_t du,scalar_t dv)
		:vs_(v1),vt_(v2),ds_(du),dt_(dv){}
		void mappingImpl(const differential_geometry_t& dg,scalar_t& s,scalar_t& t,scalar_t& dsdx,scalar_t& dtdx,
			scalar_t& dsdy,scalar_t& dtdy)const;
	private:
	vector_t vs_,vt_;
	scalar_t ds_,dt_;
};
template<typename Conf>
void SphericalMapping2D<Conf>::mappingImpl(
		const typename Conf::differential_geometry_t& dg,
		typename Conf::scalar_t& s,
		typename Conf::scalar_t& t,
		typename Conf::scalar_t& dsdx,
		typename Conf::scalar_t& dtdx,
		typename Conf::scalar_t& dsdy,
		typename Conf::scalar_t& dtdy
		)const
{
// sphere
	sphere(dg.point,s,t);
//
// texture coordinate differentials for sphere u,v
	scalar_t sx,tx,sy,ty;
	const scalar_t delta(0.1f);
	const scalar_t inv_delta(10.f);
	sphere(dg.point + delta * dg.dpdx,sx,tx);
	dsdx = (sx - s ) * (inv_delta);
	dtdx = (tx - t) * (inv_delta);
	if(dtdx > scalar_t(0.5f)) dtdx = 1 - dtdx;
	else if(dtdx < scalar_t(-0.5f)) dtdx = -(dtdx + 1);
	sphere(dg.point + delta * dg.dpdy,sy,ty);
	dsdy = (sy - s) * (inv_delta);
	dtdy = (ty - t) * inv_delta;
	if(dtdy > scalar_t(0.5f)) dtdy = 1 - dtdy;
	else if(dtdy < scalar_t(-0.5f))dtdy = -(dtdy + 1);
}
template<typename Conf>
void CylindricalMapping2D<Conf>::mappingImpl(
		const typename Conf::differential_geometry_t& dg,
		typename Conf::scalar_t& s,
		typename Conf::scalar_t& t,
		typename Conf::scalar_t& dsdx,
		typename Conf::scalar_t& dtdx,
		typename Conf::scalar_t& dsdy,
		typename Conf::scalar_t& dtdy
)const
{
	cylinder(dg.point,s,t);
	scalar_t sx,tx,sy,ty;
const scalar_t delta(.01f);
	const scalar_t inv_delta(100.f);
	cylinder(dg.point + delta * dg.dpdx, sx,tx);
	dsdx = (sx-s) * inv_delta;
	dtdx = (tx - t) * inv_delta;
	if (dtdx > scalar_t(0.5f)) dtdx = 1 - dtdx;
	else if(dtdx < scalar_t(-0.5f)) dtdx = -(dtdx + 1);
	cylinder(dg.point + delta * dg.dpdy,sy,ty);
	dsdy = (sy - s ) * inv_delta;
	dtdy = (ty - t) * inv_delta;
	if (dtdy > scalar_t(0.5f)) dtdy = 1 - dtdy;
	else if(dtdy < scalar_t(-0.5f)) dtdy = -(dtdy + 1);
	
	
}
template<typename Conf>
void PlanarMapping2D<Conf>::mappingImpl(
		const typename Conf::differential_geometry_t& dg,
		typename Conf::scalar_t& s,
		typename Conf::scalar_t& t,
		typename Conf::scalar_t& dsdx,
		typename Conf::scalar_t& dtdx,
		typename Conf::scalar_t& dsdy,
		typename Conf::scalar_t& dtdy
		)const
{
	typename Conf::vector_t vec = dg.point - typename Conf::point_t(0,0,0);
	s = ds_ + dot(vec,vs_);
	t = dt_ + dot(vec,vt_);
	dsdx = dot(dg.dpdx,vs_);
	dtdx = dot(dg.dpdx,vt_);
	dsdy = dot(dg.dpdy,vs_);
	dtdy = dot(dg.dpdy,vt_);
}

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
	ADD_CRTP_INTERFACE_TYPEDEF(color_t)
	ADD_CRTP_INTERFACE_TYPEDEF(differential_geometry_t)
	public:
	CRTP_CONST_METHOD(color_t,evaluate,1,( I_(const differential_geometry_t&,dg)))
	END_CRTP_INTERFACE

	// ConstantTexture Declarations
	template <class Conf>
	class ConstantTexture : public Texture<ConstantTexture<Conf>,Conf> {
		public:
		ADD_SAME_TYPEDEF(Conf,color_t)
		ADD_SAME_TYPEDEF(Conf,differential_geometry_t)
	public:
		// ConstantTexture Public Methods
		ConstantTexture(const color_t&v) { value = v; }
		color_t evaluateImpl(const differential_geometry_t &) const {
			return value;
		}
	private:
		color_t value;
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
		struct texturemap2d_creator
		{
			template<typename XF,typename TP>
			  typename C::texturemap2d_ptr 
			  operator()(const XF& xform,const TP& tp)const
				{
				ADD_SAME_TYPEDEF(C,texturemap2d_ptr);
				ADD_SAME_TYPEDEF(C,uv_mapping2d_t);
				ADD_SAME_TYPEDEF(C,spherical_mapping2d_t);
				ADD_SAME_TYPEDEF(C,cylindrical_mapping2d_t);
				ADD_SAME_TYPEDEF(C,planar_mapping2d_t);
				ADD_SAME_TYPEDEF(C,vector_t);
				ADD_SAME_TYPEDEF(C,scalar_t);
				texturemap2d_ptr map;
				std::string type = tp.template as<std::string>(std::string("mapping"),std::string());
				if (type == "" || type == "uv")
				{
					scalar_t su = tp.template as<scalar_t>("uscale",1);
					scalar_t sv = tp.template as<scalar_t>("vscale",1);
					scalar_t du = tp.template as<scalar_t>("udelta",0);
					scalar_t dv = tp.template as<scalar_t>("vdelta",0);
					map = new uv_mapping2d_t(su,sv,du,dv);
				}
				else if (type == "spherical")map = new spherical_mapping2d_t(xform.inverse());
				else if (type == "cylindrical")map = new cylindrical_mapping2d_t(xform.inverse());
				else if (type == "planar")
					map = new planar_mapping2d_t(
							tp.template as<vector_t>("v1",vector_t(1,0,0)),
							tp.template as<vector_t>("v2",vector_t(0,1,0)),
							tp.template as<scalar_t>("udelta",0),
							tp.template as<scalar_t>("vdelta",0));
				else map = new uv_mapping2d_t;
				return map;
				}
		};
	template<typename C>
		struct texture_creator<ConstantTexture<C> >
		{
			typedef ConstantTexture<C> texture_t;
			typedef typename texture_t::color_t color_t ;
			template<typename XF,typename TP>
			texture_t* operator()(const XF& xform,const TP& tp)const
			{
				return new texture_t(tp.template as<color_t>("value",color_t(1)));	
			}
		};
}
}
#endif
