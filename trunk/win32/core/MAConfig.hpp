#ifndef MACONFIG_HPP
#define MACONFIG_HPP

#include <boost/config.hpp>

static const char* ma_version = "0.000000001";
//compiler configure

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
#define MA_WINDOWS_
#define MA_WINDOWS_API_
#ifndef MA_USE_SDL_DEVICE_
#define MA_USE_WINDOWS_DEVICE_
#endif
#else

#define MA_X11_DEVICE

#endif


#define MA_FAST_MATH

#endif
