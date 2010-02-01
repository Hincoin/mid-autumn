#include "script.hpp"
#include "MAAPI.hpp"



namespace OOLUA{
// API Function Declarations
LUA_EXPORT_FUNC(void(),maIdentity)
LUA_EXPORT_FUNC(void (float,float,float),maTranslate)
LUA_EXPORT_FUNC(void (float,float,float,float),maRotate)
LUA_EXPORT_FUNC(void (float,float,float),maScale)
LUA_EXPORT_FUNC(void (float,float,float, float,float,float, float,float,float),maLookAt)

static int l_maCoordinateSystem(lua_State* l)
{
	const char* s = lua_tostring(l,1);
	maCoordinateSystem(s);
	return 0;
}

static int l_maCoordSysTransform(lua_State* l)
{
	const char* s = lua_tostring(l,1);
	maCoordSysTransform(s);
}
//extern COREDLL void maConcatTransform(float transform[16]);
//extern COREDLL void maTransform(float transform[16]);
//LUA_EXPORT_FUNC(void(const char*),maCoordinateSystem);
//LUA_EXPORT_FUNC(void(const char*),maCoordSysTransform);
//extern COREDLL void maPixelFilter(const std::string &name, const ParamSet &params);
//extern COREDLL void maFilm(const std::string &type,
//							 const ParamSet &params);
//extern COREDLL void maSampler(const std::string &name,
//								const ParamSet &params);
//extern COREDLL void maAccelerator(const std::string &name,
//									const ParamSet &params);
//extern COREDLL
//void maSurfaceIntegrator(const std::string &name,
	//					   const ParamSet &params);
}

void register_api(lua_State* l)
{
	using namespace OOLUA;
	REGISTER_FUNC(l,maIdentity)
	REGISTER_FUNC(l,maTranslate)
	REGISTER_FUNC(l,maRotate)
	REGISTER_FUNC(l,maScale)
	REGISTER_FUNC(l,maLookAt)
	REGISTER_FUNC(l,maCoordinateSystem)
	REGISTER_FUNC(l,maCoordSysTransform)
}


void register_all_class(lua_State* l)
{
}

