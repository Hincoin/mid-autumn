#ifndef MA_INCLUDED_WRITERGBAIMAGE_HPP
#define MA_INCLUDED_WRITERGBAIMAGE_HPP

#include <string>
namespace ma{
	extern void WriteRGBAImage(std::string&,float* rgb,float* alhpa,int x_pixel_count,int y_pixel_count,int x_res,int y_res,
		int x_pixel_start,int y_pixel_start);
}

#endif