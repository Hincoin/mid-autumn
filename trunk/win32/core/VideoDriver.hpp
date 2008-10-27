#ifndef INCLUDE_VIDEO_DRIVER_HPP
#define INCLUDE_VIDEO_DRIVER_HPP

#include "CRTPInterfaceMacro.hpp"
#include "SpaceSegment.hpp"

namespace ma{
	MA_DECLARE_TYPEDEF_TRAITS_TYPE(Color)

	BEGIN_CRTP_INTERFACE(VideoDriver)
		ADD_CRTP_INTERFACE_TYPEDEF(Color)
		ADD_CRTP_INTERFACE_FUNC(bool,beginScene,(bool backBuffer,bool zBuffer,Color clr),(backBuffer,zBuffer,clr),)
		ADD_CRTP_INTERFACE_FUNC(bool,endScene,(int windowid,recti* sourceRect),(windowid, sourceRect),)	
	END_CRTP_INTERFACE()
}
#endif