#ifndef MA_VIDEOMODES_HPP
#define MA_VIDEOMODES_HPP


#include "Vector.hpp"
namespace ma{
	template<typename Derived>
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
		rangei  getVideoModeResolution (int modeNumber) const 
		{
			return derived().getVideoModeResolution(modeNumber);
		}
			
	};
}
#endif