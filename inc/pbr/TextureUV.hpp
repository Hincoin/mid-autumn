
#ifndef _MA_INCLUDED_TEXTUREUV_HPP_
#define _MA_INCLUDED_TEXTUREUV_HPP_
#include "Texture.hpp"

namespace ma{
	// UVTexture Declarations
	template<typename Conf>
	class UVTexture : public Texture<UVTexture<Conf>,Conf> {
		public:
		ADD_SAME_TYPEDEF(Conf,texturemap2d_ptr);
		ADD_SAME_TYPEDEF(Conf,differential_geometry_t);
		ADD_SAME_TYPEDEF(Conf,color_t);
		ADD_SAME_TYPEDEF(Conf,scalar_t);
	public:
		// UVTexture Public Methods
		UVTexture(texturemap2d_ptr m) {
			mapping = m;
		}
		~UVTexture() {
			delete_ptr(mapping);
		}
		color_t evaluateImpl(const differential_geometry_t &dg) const {
			scalar_t s, t, dsdx, dtdx, dsdy, dtdy;
			texture::mapping((mapping),dg, ref(s), ref(t), ref(dsdx), ref(dtdx), ref(dsdy), ref(dtdy));
			scalar_t cs[color_t::COLOR_SAMPLES];
			memset(cs, 0, color_t::COLOR_SAMPLES * sizeof(scalar_t));
			cs[0] = s - floor32(s);
			cs[1] = t - floor32(t);
			return color_t(cs);
		}
	private:
		texturemap2d_ptr mapping;
	};
}

namespace ma{
namespace details
{
	template<typename C>
		struct texture_creator<UVTexture<C> >
		{
			typedef UVTexture<C> texture_t;
			typedef typename texture_t::scalar_t scalar_t;
			template<typename XF,typename TP>
			texture_t* operator()(const XF& xform,const TP& tp)const
			{
				ADD_SAME_TYPEDEF(C,texturemap2d_ptr);
				ADD_SAME_TYPEDEF(C,uv_mapping2d_t);
				ADD_SAME_TYPEDEF(C,spherical_mapping2d_t);
				ADD_SAME_TYPEDEF(C,cylindrical_mapping2d_t);
				ADD_SAME_TYPEDEF(C,planar_mapping2d_t);
				ADD_SAME_TYPEDEF(C,vector_t);
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

				return new texture_t(map);

			}
		};
}
MAKE_TYPE_STR_MAP(1,UVTexture,uv)

}
#endif
