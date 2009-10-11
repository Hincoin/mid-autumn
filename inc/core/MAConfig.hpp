#ifndef MACONFIG_HPP
#define MACONFIG_HPP

#include <boost/config.hpp>

//extern const char* ma_version ;//= "0.000000001";
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

#include <cassert>
#define MA_ASSERT(X) assert(X);


#ifdef WIN32
#ifdef CORE_SOURCE
#define COREDLL __declspec(dllexport)
#else
#define COREDLL __declspec(dllimport)
#endif
#define DLLEXPORT __declspec(dllexport)
#else
#define COREDLL
#define DLLEXPORT
#endif


#define TBB_PARALLEL
//#define OMP_PARALLEL

//define max number of threads 
#ifdef TBB_PARALLEL
#define MAX_PARALLEL 64
#else
#define MAX_PARALLEL 1
#endif

#ifdef TBB_PARALLEL
#include <tbb/tbb.h>
#endif

#endif
