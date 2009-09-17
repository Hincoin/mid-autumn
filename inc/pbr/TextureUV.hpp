
#ifndef _MA_INCLUDED_TEXTUREUV_HPP_
#define _MA_INCLUDED_TEXTUREUV_HPP_
#include "Texture.hpp"

namespace ma{
	// UVTexture Declarations
	template<typename Conf>
	class UVTexture : public Texture<UVTexture<Conf>,Conf> {
		ADD_SAME_TYPEDEF(Conf,texturemap2d_ptr);
		ADD_SAME_TYPEDEF(Conf,differential_geometry_t);
		ADD_SAME_TYPEDEF(Conf,spectrum_t);
		ADD_SAME_TYPEDEF(Conf,scalar_t);
	public:
		// UVTexture Public Methods
		UVTexture(texturemap2d_ptr m) {
			mapping = m;
		}
		~UVTexture() {
			delete_ptr(mapping);
		}
		spectrum_t evaluateImpl(const differential_geometry_t &dg) const {
			scalar_t s, t, dsdx, dtdx, dsdy, dtdy;
			mapping->map(dg, s, t, dsdx, dtdx, dsdy, dtdy);
			scalar_t cs[COLOR_SAMPLES];
			memset(cs, 0, COLOR_SAMPLES * sizeof(scalar_t));
			cs[0] = s - Floor2Int(s);
			cs[1] = t - Floor2Int(t);
			return spectrum_t(cs);
		}
	private:
		texturemap2d_ptr mapping;
	};
}
#endif