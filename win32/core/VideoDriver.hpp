#ifndef INCLUDE_VIDEO_DRIVER_HPP
#define INCLUDE_VIDEO_DRIVER_HPP

#include "CRTPInterfaceMacro.hpp"
#include "SpaceSegment.hpp"

namespace ma{

	BEGIN_CRTP_INTERFACE(VideoDriver)
		ADD_CRTP_INTERFACE_TYPEDEF(Color)
		ADD_CRTP_INTERFACE_FUNC(bool,beginScene,(bool backBuffer,bool zBuffer,Color clr),(backBuffer,zBuffer,clr),)
		ADD_CRTP_INTERFACE_FUNC(template<typename PresenterPtr> bool,endScene,(PresenterPtr presenter,int windowid,recti* sourceRect),(presenter,windowid, sourceRect), )
	END_CRTP_INTERFACE()


	template<typename CreateConfig>
	struct CreateDriver;
}
#endif