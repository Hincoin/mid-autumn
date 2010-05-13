#ifndef MA_VIDEOMODES_HPP
#define MA_VIDEOMODES_HPP

#include "VideoModes.hpp"

namespace ma{

	class MAVideoModes:public VideoModes<MAVideoModes> {
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

        void addMode(const scalar2i& size,int depth)
        {}
        void setDesktop(int depth,const scalar2i& size)
        {}
	};
}
#endif
