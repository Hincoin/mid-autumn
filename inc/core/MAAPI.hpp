#ifndef _MA_INCLUDE_MAAPI_HPP_
#define _MA_INCLUDE_MAAPI_HPP_

// api.h*
#include "MAConfig.hpp"
#include <string>
#include "ParamSet.hpp"

// API Function Declarations
extern COREDLL void maIdentity();
extern COREDLL void maTranslate(float dx, float dy, float dz);
extern COREDLL void maRotate(float angle,
							   float ax,
							   float ay,
							   float az);
extern COREDLL void maScale(float sx,
							  float sy,
							  float sz);
extern COREDLL void maLookAt(float ex,
							   float ey,
							   float ez,
							   float lx,
							   float ly,
							   float lz,
							   float ux,
							   float uy,
							   float uz);
extern COREDLL
void maConcatTransform(float transform[16]);
extern COREDLL
void maTransform(float transform[16]);
extern COREDLL void maCoordinateSystem(const std::string &);
extern COREDLL void maCoordSysTransform(const std::string &);
extern COREDLL void maPixelFilter(const std::string &name, const ParamSet &params);
extern COREDLL void maFilm(const std::string &type,
							 const ParamSet &params);
extern COREDLL void maSampler(const std::string &name,
								const ParamSet &params);
extern COREDLL void maAccelerator(const std::string &name,
									const ParamSet &params);
extern COREDLL
void maSurfaceIntegrator(const std::string &name,
						   const ParamSet &params);
extern COREDLL
void maVolumeIntegrator(const std::string &name,
						  const ParamSet &params);
extern COREDLL void maCamera(const std::string &, const ParamSet &cameraParams);
extern COREDLL void maSearchPath(const std::string &path);
extern COREDLL void maFrameBegin(const std::string&,const ParamSet& frameParams);
extern COREDLL void maWorldBegin();
extern COREDLL void maAttributeBegin();
extern COREDLL void maAttributeEnd();
extern COREDLL void maTransformBegin();
extern COREDLL void maTransformEnd();
extern COREDLL void maTexture(const std::string &name, const std::string &type,
								const std::string &texname, const ParamSet &params);
extern COREDLL void maMaterial(const std::string &name,
								 const ParamSet &params);
extern COREDLL void maLightSource(const std::string &name, const ParamSet &params);
extern COREDLL void maAreaLightSource(const std::string &name, const ParamSet &params);
extern COREDLL void maShape(const std::string &name, const ParamSet &params);
extern COREDLL void maReverseOrientation();
extern COREDLL void maVolume(const std::string &name, const ParamSet &params);
extern COREDLL void maObjectBegin(const std::string &name);
extern COREDLL void maObjectEnd();
extern COREDLL void maObjectInstance(const std::string &name);
extern COREDLL void maWorldEnd();
extern COREDLL void maFrameEnd();

extern COREDLL void maInit();
extern COREDLL void maCleanUp();
#endif