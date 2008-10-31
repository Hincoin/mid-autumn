#ifndef MACONFIG_HPP
#define MACONFIG_HPP


static const char* ma_version = "0.000000001";

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
#define MA_WINDOWS_
#define MA_WINDOWS_API_
#ifndef MA_USE_SDL_DEVICE_
#define MA_USE_WINDOWS_DEVICE_
#endif


#define MA_FAST_MATH


#endif



#endif