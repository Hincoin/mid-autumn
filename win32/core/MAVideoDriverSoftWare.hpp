#ifndef MA_VIDEODRIVER_SOFTWARE_HPP
#define MA_VIDEODRIVER_SOFTWARE_HPP

#include "VideoDriver.hpp"

namespace ma{
	template<typename Configure>
	class MAVideoDriverSoftWare:public VideoDriver<MAVideoDriverSoftWare<Configure> >{
		//
	};

	struct default_video_driver_software_config{};
}
#endif