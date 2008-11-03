#ifndef MA_VIDEODRIVER_SOFTWARE_IMPL_HPP
#define MA_VIDEODRIVER_SOFTWARE_IMPL_HPP

#include "MAVideoDriverSoftWare.hpp"

namespace ma{
	template<typename Configure>
	MAVideoDriverSoftWare<Configure>::MAVideoDriverSoftWare
		(const scalar2i& windowSize,bool fullscreen, FileSystemPtr fs):
	ScreenSize(windowSize),FileSystem_(fs)
	{
		BackBuffer = new Image(ECF_A1R5G5B5, windowSize);
		BackBuffer->fill(Color(0));
	}
	template<typename Configure>
	MAVideoDriverSoftWare<Configure>::~MAVideoDriverSoftWare()
	{
		//delete pointer free memory
	}
}


#endif