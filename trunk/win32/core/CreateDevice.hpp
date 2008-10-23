#ifndef MA_CREATE_DEVICE_HPP
#define MA_CREATE_DEVICE_HPP

#include "SpaceSegment.hpp"
#include "Vector.hpp"

namespace ma{


	template <typename Config>
	struct CreateDevice{
		typename Config::DevicePtr operator()(const scalar2i& window_size,unsigned int bits,bool fullscreen,
			bool stencil_buffer,bool vsync,typename Configure::EventProcessorPtr evt){
				typedef typename Configure::DeviceType;
				return new DeviceType(window_size,bits,fullscreen,stencil_buffer,vsync,evt);
		}
	};
}
#endif