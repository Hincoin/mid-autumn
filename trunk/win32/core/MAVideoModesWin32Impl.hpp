#ifndef MA_VIDEOMODES_WIN32IMPL_HPP
#define MA_VIDEOMODES_WIN32IMPL_HPP

#include "VideoModes.hpp"

namespace ma{

	class MAVideoModesWin32:public VideoModes<MAVideoModesWin32> {
	public:
		//Get the pixel depth of a video mode in bits. 
		int  getDesktopDepth () const 
		{return 0;}

		//Get current desktop screen resolution. 
		scalar2i getDesktopResolution () const 
		{
			return scalar2i();
		}
		//Gets amount of video modes in the list. 
		int  getVideoModeCount () const {return 0;} 

		//Get the pixel depth of a video mode in bits. 
		int  getVideoModeDepth (int modeNumber) const 
		{return 0;}; 

		//Get the screen size of a video mode in pixels. 
		scalar2i  getVideoModeResolution (int modeNumber) const 
		{
			return scalar2i();
		}

	};
}
#endif 