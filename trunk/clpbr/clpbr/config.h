#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef CL_KERNEL
#define GLOBAL __global 
#define CONSTANT __global //__constant 
#define LOCAL //__local 

#define INLINE 

// NOTE: workaround for an Apple OpenCL compiler bug
#ifdef __APPLE__
#define OCL_CONSTANT_BUFFER __global
#else
#define OCL_CONSTANT_BUFFER __constant
#endif

#else
#define GLOBAL
#define CONSTANT 
#define LOCAL 
#define OCL_CONSTANT_BUFFER
#define INLINE inline

#define as_uint(_i) (*(unsigned*)(&(_i)))
#define as_float(_f) (*(float*)(&(_f)))

#endif


#endif
