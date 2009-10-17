
#ifdef _MSC_VER
#pragma   warning(push)
#pragma warning( disable : 4181 4244 4127 4211)
#endif


#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include "MAConfig.hpp"

#if ( _MSC_VER <= 1400 ) //msvc 2005
#define EIGEN_DONT_ALIGN
#endif

#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/QR>
#include <Eigen/Geometry>
#include <Eigen/Array>

#ifdef _MSC_VER
#pragma   warning(pop)
#endif
