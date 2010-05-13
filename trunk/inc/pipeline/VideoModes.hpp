#ifndef INCLUDE_VIDEOMODES_HPP
#define INCLUDE_VIDEOMODES_HPP


#include "Vector.hpp"
#include "NullType.hpp"

namespace ma{
	template<typename Derived,typename ClassConfig = NullType>
	class VideoModes{
	private:
		VideoModes& derived(){static_cast<VideoModes&>(*this);}
		const VideoModes& derived()const{static_cast<const VideoModes&>(*this);}

	protected:
		~VideoModes(){}
	public:
		//Get the pixel depth of a video mode in bits.
		int  getDesktopDepth () const
		{return derived().getDesktopDepth();}

		//Get current desktop screen resolution.
		scalar2i getDesktopResolution () const
		{
			return derived().getDesktopResolution();
		}
		//Gets amount of video modes in the list.
		int  getVideoModeCount () const {return derived().getVideoModeCount();}

		//Get the pixel depth of a video mode in bits.
		int  getVideoModeDepth (int modeNumber) const {return derived().getVideoModeDepth(modeNumber);};

		//Get the screen size of a video mode in pixels.
		scalar2i  getVideoModeResolution (int modeNumber) const
		{
			return derived().getVideoModeResolution(modeNumber);
		}
				//! adds a new mode to the list
		void addMode(const scalar2i& size, int depth){
		    return derived().addMode(size,depth);
		    }

		void setDesktop(int desktopDepth, const scalar2i& desktopSize)
		{
		    return derived().setDesktop(desktopDepth,desktopSize);
        }

	};
}
#endif
