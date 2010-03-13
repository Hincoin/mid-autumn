#include "script.hpp"
#include "MAAPI.hpp"
#include "MAny.hpp"


namespace OOLUA{
// API Function Declarations
LUA_EXPORT_FUNC(void(),maIdentity)
LUA_EXPORT_FUNC(void (float,float,float),maTranslate)
LUA_EXPORT_FUNC(void (float,float,float,float),maRotate)
LUA_EXPORT_FUNC(void (float,float,float),maScale)
LUA_EXPORT_FUNC(void (float,float,float, float,float,float, float,float,float),maLookAt)
LUA_EXPORT_FUNC(void(),maInit);
LUA_EXPORT_FUNC(void(),maCleanUp);
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
	return 0;
}
//extern COREDLL void maConcatTransform(float transform[16]);
static int l_maConcatTransform(lua_State* l)
{
		float trans[4][4];
		for(int i = 0,lua_pushnil(l);(i < 16) && lua_next(l,-2) ;++i , lua_pop(l,1))
		{
			trans[i/4][i%4]=(float)lua_tonumber(l,-1);
		}
		maConcatTransform(trans);
		return 0;
}
//extern COREDLL void maTransform(float transform[16]);
static int l_maTransform(lua_State* l)
{
		float trans[4][4];
		for(int i = 0,lua_pushnil(l);(i < 16) && lua_next(l,-2) ;++i , lua_pop(l,1))
		{
			trans[i/4][i%4]=(float)lua_tonumber(l,-1);
		}
		maTransform(trans);
		return 0;
}
//LUA_EXPORT_FUNC(void(const char*),maCoordinateSystem);
//LUA_EXPORT_FUNC(void(const char*),maCoordSysTransform);
//extern COREDLL void maPixelFilter(const std::string &name, const ParamSet &params);

static ParamSet get_params(lua_State* l)
{
	ParamSet params;
	for(lua_pushnil(l);lua_next(l,-2);lua_pop(l,1))
	{
		std::string key_name = lua_tolstring(l,-2,0);
		ma::MAny val;
		printf("key:%s \t",key_name.c_str());
		switch(lua_type(l,-1))
		{
			case LUA_TNIL:break;
			case LUA_TNUMBER:
						  params.add(key_name,(double)lua_tonumber(l,-1));
						  break;
			case LUA_TBOOLEAN:
						  params.add(key_name,(bool)lua_toboolean(l,-1));
						  break;
			case LUA_TSTRING:
						  params.add(key_name,std::string(lua_tolstring(l,-1,0)));
						  break;
			case LUA_TTABLE:
						  {
							  params.add(key_name,get_params(l));
						  }
						  break;
			case LUA_TFUNCTION:break;
			case LUA_TUSERDATA:
							  params.add(key_name,lua_touserdata(l,-1)); 
							   break;
			case LUA_TTHREAD:break;
			case LUA_TLIGHTUSERDATA:
							 params.add(key_name,lua_touserdata(l,-1));
							 break;
			default:break;
		}
	}
	return params;
}
static int l_maPixelFilter(lua_State* l)
{
	luaL_checktype(l,1,LUA_TSTRING);
	luaL_checktype(l,2,LUA_TTABLE);
	std::string name = lua_tolstring(l,1,0);
	printf("name %s \n",name.c_str());
	ParamSet params(get_params(l));
	/*
	for(lua_pushnil(l);lua_next(l,-2);lua_pop(l,1))
	{
		std::string key_name = lua_tolstring(l,-2,0);
		MAny val;
		printf("key:%s \t",key_name.c_str());
		switch(lua_type(l,-1))
		{
			case LUA_TNIL:break;
			case LUA_TNUMBER:
						  val = (double)lua_tonumber(l,-1);
						  break;
			case LUA_TBOOLEAN:
						  val = (bool)lua_toboolean(l,-1);
						  break;
			case LUA_TSTRING:
						  val = std::string(lua_tolstring(l,-1,0));
						  break;
			case LUA_TTABLE:break;
			case LUA_TFUNCTION:break;
			case LUA_TUSERDATA:break;
			case LUA_TTHREAD:break;
			case LUA_TLIGHTUSERDATA:break;
			default:break;
		}
		params.add(key_name,val);
	}*/
	printf("enter filter function");
	maPixelFilter(name,params);
	return 0;
}
//extern COREDLL void maFilm(const std::string &type,
//							 const ParamSet &params);
static int l_maFilm(lua_State* l)
{
	return 0;
}
//extern COREDLL void maSampler(const std::string &name,
//								const ParamSet &params);
static int l_maSampler(lua_State* l)
{

	return 0;
}	
//extern COREDLL void maAccelerator(const std::string &name,
//									const ParamSet &params);
static int l_maAccelerator(lua_State* l)
{

	return 0;
}
//extern COREDLL
//void maSurfaceIntegrator(const std::string &name,
	//					   const ParamSet &params);
static int l_maSurfaceIntegrator(lua_State* l)
{

	return 0;
}

}

void register_api(lua_State* l)
{
	using namespace OOLUA;
	REGISTER_FUNC(l,maInit);
	REGISTER_FUNC(l,maCleanUp);
	REGISTER_FUNC(l,maIdentity)
	REGISTER_FUNC(l,maTranslate)
	REGISTER_FUNC(l,maRotate)
	REGISTER_FUNC(l,maScale)
	REGISTER_FUNC(l,maLookAt)
	REGISTER_FUNC(l,maCoordinateSystem)
	REGISTER_FUNC(l,maCoordSysTransform)
	REGISTER_FUNC(l,maPixelFilter);
	REGISTER_FUNC(l,maFilm);
	REGISTER_FUNC(l,maSampler);
	REGISTER_FUNC(l,maAccelerator);
	REGISTER_FUNC(l,maSurfaceIntegrator);
}


void register_all_class(lua_State* l)
{
}

