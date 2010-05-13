#ifndef _MA_INCLUDED_TEXTURE_CHECKBOARD_HPP_
#define _MA_INCLUDED_TEXTURE_CHECKBOARD_HPP_ 
#include "Texture.hpp"
#include "ErrorReporter.hpp"
namespace ma
{

	template<typename Conf>
		class CheckerBoard2D:public Texture<CheckerBoard2D<Conf>,Conf>
	{
		public:
		ADD_SAME_TYPEDEF(Conf,texturemap2d_ptr)
		ADD_SAME_TYPEDEF(Conf,texture_ref_t);
		ADD_SAME_TYPEDEF(Conf,color_t);
		ADD_SAME_TYPEDEF(Conf,scalar_t);

		ADD_SAME_TYPEDEF(Conf,differential_geometry_t);

		texturemap2d_ptr mapping_;
		texture_ref_t tex1_,tex2_;
		enum Anti_AliasingType{NONE,SUPERSAMPLE,CLOSEDFORM};
		Anti_AliasingType aa_method_;
		public:
			CheckerBoard2D(texturemap2d_ptr m,
					texture_ref_t c1,
					texture_ref_t c2,
					const std::string& aa):mapping_(m),tex1_(c1),tex2_(c2)
		{
			assert(mapping_);
			assert(tex1_);
			assert(tex2_);
			if (aa == "none") aa_method_ = NONE;
			else if(aa == "supersample")aa_method_ = SUPERSAMPLE;
			else if (aa == "closedform") aa_method_ = CLOSEDFORM;
			else {
				report_warning("Unkown Anti-Aliasing mode %s used by CheckerBoard2D,default to \"supersample\"",aa.c_str());
				aa_method_ = SUPERSAMPLE;
			}
		}
			scalar_t bumpint(scalar_t x)const
			{
				return (floor32(x/2) + 2 * std::max((x/2) - floor32(x/2) - 0.5f,0.f));
			}
			~CheckerBoard2D(){ delete_ptr(mapping_);}
			color_t evaluateImpl(const differential_geometry_t& dg)const
			{
				scalar_t s(0),t(0),dsdx(0),dtdx(0),dsdy(0),dtdy(0);
				texture::mapping(mapping_,dg,ref(s),ref(t),ref(dsdx),ref(dtdx),ref(dsdy),ref(dtdy));
				if(aa_method_ == CLOSEDFORM)
				{
					//compute closed form box-filtered _CheckerBoard2D value
					//evaluate single check if filter is entirly inside one of them
					scalar_t ds = std::max(std::abs(dsdx),std::abs(dsdy));
					scalar_t dt = std::max(std::abs(dtdx),std::abs(dtdy));

					scalar_t s0 = s - ds;
					scalar_t s1=s+ds;
					scalar_t t0 = t - dt;
					scalar_t t1 = t + dt;
					if(floor32(s0) == floor32(s1) && 
							floor32(t0) == floor32(t1))
					{
						if ((floor32(s) + floor32(t)) % 2 == 0)
							return texture::evaluate(tex1_,dg);
						return texture::evaluate(tex2_,dg);
					}
					scalar_t sint = (bumpint(s1) - bumpint(s0)) * reciprocal(2 * ds);
					scalar_t tint = (bumpint(t1) - bumpint(t0)) * reciprocal(2 * dt);
					scalar_t area2 = sint + tint - 2 * sint * tint;
					if (ds > 1 || dt > 1)
						area2 = scalar_t(0.5f);
					return (1.f - area2) * texture::evaluate(tex1_,dg) + area2 * texture::evaluate(tex2_,dg);
				}
				else if(aa_method_ == SUPERSAMPLE)
				{
					//todo
				}
				if((floor32(s) + floor32(t)) % 2 == 0)
					return texture::evaluate(tex1_,dg);
				return texture::evaluate(tex2_,dg);
			}
	};

}

namespace ma
{
	MAKE_TYPE_STR_MAP(1,CheckerBoard2D,checker);
namespace details
{

	template<typename C>
		struct texture_creator<CheckerBoard2D<C> >
		{
			typedef CheckerBoard2D<C> texture_t;
			typedef typename C::spectrum_t spectrum_t;
			typedef typename texture_t::scalar_t scalar_t;
			ADD_SAME_TYPEDEF(C,color_t)
			ADD_SAME_TYPEDEF(C,texture_ref_t);	
			template<typename TP>
			texture_ref_t get_texture(const TP& tp,const std::string& name,const spectrum_t& def)const
			{
				return tp.getSpectrumTexture(name,def);
			}	
			template<typename TP>
			texture_ref_t get_texture(const TP& tp,const std::string& name,const scalar_t& def)const
			{
				return tp.getFloatTexture(name,def);
			}	
			template<typename XF,typename TP>
			texture_t* operator()(const XF& xform,const TP& tp)const
			{
				texture_ref_t t1; 
			   	t1 = get_texture(tp,"tex1",color_t(1));
				texture_ref_t t2;
			    t2	= get_texture(tp,"tex2",color_t(0));
				std::string aa_method = tp.template as<std::string>("aamode",std::string("closedform"));
				return new texture_t(texturemap2d_creator<C>()(xform,tp),t1,t2,aa_method);
			}
		};

}

}
#endif
