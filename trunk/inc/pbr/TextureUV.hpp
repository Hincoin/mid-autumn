
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
				return new texture_t(texturemap2d_creator<C>()(xform,tp));

			}
		};
}
MAKE_TYPE_STR_MAP(1,UVTexture,uv)

}
#endif
