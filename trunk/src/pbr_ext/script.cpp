#include "script.hpp"
#include "MAAPI.hpp"
#include "MAny.hpp"

#include "DefaultConfigurations.hpp"
namespace OOLUA{
	using namespace ma;
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


//only single thread allowed here
static ParamSet static_param;
using namespace ma;
static int l_maBeginParam(lua_State* l)
{
	assert(static_param.empty());
	return 0;
}
static int l_maAddPointParam(lua_State* l)
{
	luaL_checktype(l,1,LUA_TSTRING);
	luaL_checktype(l,2,LUA_TTABLE);
	std::string name = lua_tolstring(l,1,0);
	basic_config<>::scalar_t numbers[3];
	int i = 0;
	for(lua_pushnil(l);lua_next(l,-2) && i < 3;lua_pop(l,1))
	{
		luaL_checktype(l,-2,LUA_TNUMBER);
		switch(lua_type(l,-1))
		{
			case LUA_TNUMBER:
				numbers[i]=(float)lua_tonumber(l,-1);
				break;
			default:assert(false);break;
		}
		i++;
	}
	assert(i == 3);
	static_param.add(name,basic_config<>::point_t(numbers[0],numbers[1],numbers[2]));
	return 0;
}
static int l_maAddSpectrumParam(lua_State* l)
{
	luaL_checktype(l,1,LUA_TSTRING);
	luaL_checktype(l,2,LUA_TTABLE);
	std::string name = lua_tolstring(l,1,0);
	basic_config<>::scalar_t numbers[3];
	int i = 0;
	for(lua_pushnil(l);lua_next(l,-2) && i < 3;lua_pop(l,1))
	{
		luaL_checktype(l,-2,LUA_TNUMBER);
		switch(lua_type(l,-1))
		{
			case LUA_TNUMBER:
				numbers[i]=(float)lua_tonumber(l,-1);
				break;
			default:assert(false);break;
		}
		i++;
	}
	assert(i == 3 || i == 1);
	printf("add spectrum param %d (%f,%f,%f)\n",i,numbers[0],numbers[1],numbers[2]);
	if ( i == 1)
		static_param.add(name,basic_config<>::spectrum_t(numbers[0]));
	else if ( i == 3) 
		static_param.add(name,basic_config<>::spectrum_t(numbers));
	return 0;
}
static int l_maAddPointArrayParam(lua_State* l)
{
	printf("point arra xxxxxxxxxxxxxxxxxxxxxxxxxxxx ");
	luaL_checktype(l,1,LUA_TSTRING);
	luaL_checktype(l,2,LUA_TTABLE);
	std::string name = lua_tolstring(l,1,0);
	std::vector<basic_config<>::point_t> point_array;
	printf("point arra  aaaaaaaaaaaaaaaaaaaaa");

	for(lua_pushnil(l);lua_next(l,-2);lua_pop(l,1))
	{
		luaL_checktype(l,-2,LUA_TNUMBER);
		luaL_checktype(l,-1,LUA_TTABLE);
		switch(lua_type(l,-1))
		{
			case LUA_TTABLE:
				{
					basic_config<>::scalar_t numbers[3];
					int i = 0;
					for(lua_pushnil(l);lua_next(l,-2) && i < 3;lua_pop(l,1))
					{
						luaL_checktype(l,-2,LUA_TNUMBER);//key is number
						luaL_checktype(l,-1,LUA_TNUMBER);//value is number
						switch(lua_type(l,-1))
						{
							case LUA_TNUMBER:
								numbers[i]=(float)lua_tonumber(l,-1);
								break;
							default:assert(false);break;
						}
					i++;
					}
					point_array.push_back(basic_config<>::point_t(numbers[0],numbers[1],numbers[2]));
				}
				break;
			default:assert(false);break;
		}
	}
	static_param.add(name,point_array);
	return 0;
}
//number array
#define ADD_NUMERIC_ARRAY(Name,T)\
static int l_maAdd##Name##ArrayParam(lua_State* l)\
{\
	luaL_checktype(l,1,LUA_TSTRING);\
	luaL_checktype(l,2,LUA_TTABLE);\
	std::string name = lua_tolstring(l,1,0);\
	std::vector<T> numbers;\
	for(lua_pushnil(l);lua_next(l,-2);lua_pop(l,1))\
	{\
		luaL_checktype(l,-2,LUA_TNUMBER);\
		switch(lua_type(l,-1))\
		{\
			case LUA_TNUMBER:\
				numbers.push_back((T)lua_tonumber(l,-1));\
				break;\
			default:assert(false);break;\
		}\
	}\
	static_param.add(name,numbers);\
	return 0;\
}\

ADD_NUMERIC_ARRAY(Float,float)
ADD_NUMERIC_ARRAY(Int,int)
ADD_NUMERIC_ARRAY(Unsigned,unsigned int)
//etc
//

static int l_maEndParam(lua_State* l)
{
#ifdef _DEBUG
	std::cerr<<"parameter added"<<"\t";
	for(ParamSet::iterator it = static_param.begin();it != static_param.end();++it)
		std::cerr<<it->first<<std::endl;	
#endif
	assert(!static_param.empty());
	static_param.clear();
	return 0;
}
static ParamSet get_params(lua_State* l)
{
	if(! static_param.empty()) return static_param;
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
#define CALL_API(L,FUNC)\
	luaL_checktype(L,1,LUA_TSTRING);\
	luaL_checktype(L,2,LUA_TTABLE);\
	std::string name = lua_tolstring(l,1,0);\
	ParamSet params(get_params(l));\
	FUNC(name,params);\

#define DEF_LUA_API_FUNC(FUNC)\
	static int l_##FUNC(lua_State* l)\
{\
	CALL_API(l,FUNC)\
	return 0;\
}
DEF_LUA_API_FUNC(maPixelFilter);
DEF_LUA_API_FUNC(maFilm)
DEF_LUA_API_FUNC(maSampler)
DEF_LUA_API_FUNC(maAccelerator)
DEF_LUA_API_FUNC(maSurfaceIntegrator)
DEF_LUA_API_FUNC(maVolumeIntegrator)
DEF_LUA_API_FUNC(maCamera)
DEF_LUA_API_FUNC(maFrameBegin)
//DEF_LUA_API_FUNC(maTexture)
DEF_LUA_API_FUNC(maMaterial)
DEF_LUA_API_FUNC(maLightSource)
DEF_LUA_API_FUNC(maAreaLightSource)
DEF_LUA_API_FUNC(maShape)
DEF_LUA_API_FUNC(maVolume)

LUA_EXPORT_FUNC(void(),maWorldBegin)
LUA_EXPORT_FUNC(void(),maAttributeBegin)
LUA_EXPORT_FUNC(void(),maAttributeEnd)
LUA_EXPORT_FUNC(void(),maTransformBegin)
LUA_EXPORT_FUNC(void(),maTransformEnd)
LUA_EXPORT_FUNC(void(),maReverseOrientation)
//LUA_EXPORT_FUNC(void(const char*),maObjectBegin)
static int l_maObjectBegin(lua_State* l)
{
	luaL_checktype(l,1,LUA_TSTRING);
	std::string str = lua_tolstring(l,1,0);
	maObjectBegin(str);
	return 0;
}
LUA_EXPORT_FUNC(void(),maObjectEnd)
//LUA_EXPORT_FUNC(void(const char*),maObjectInstance)
static int l_maObjectInstance(lua_State* l)
{
	luaL_checktype(l,1,LUA_TSTRING);
	maObjectInstance(lua_tolstring(l,1,0));
	return 0;
}
LUA_EXPORT_FUNC(void(),maWorldEnd)
LUA_EXPORT_FUNC(void(),maFrameEnd)
//extern COREDLL void maTexture(const std::string &name, const std::string &type,
//								const std::string &texname, const ParamSet &params);
//
static int l_maTexture(lua_State* l)
{
	luaL_checktype(l,1,LUA_TSTRING);
	luaL_checktype(l,2,LUA_TSTRING);
	luaL_checktype(l,3,LUA_TSTRING);
	std::string name = lua_tostring(l,1);
	std::string type = lua_tostring(l,2);
	std::string texname = lua_tostring(l,3);
	
	ParamSet p(get_params(l));
	maTexture(name,type,texname,p);	
	return 0;
}
//end of api
///////////////////////////////////////////////////////////////////////////////////////
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
	REGISTER_FUNC(l,maVolumeIntegrator)
	REGISTER_FUNC(l,maCamera)
	REGISTER_FUNC(l,maFrameBegin)
	REGISTER_FUNC(l,maMaterial)
	REGISTER_FUNC(l,maLightSource)
	REGISTER_FUNC(l,maAreaLightSource)
	REGISTER_FUNC(l,maShape)
	REGISTER_FUNC(l,maVolume)

	REGISTER_FUNC(l,maWorldBegin)
	REGISTER_FUNC(l,maAttributeBegin)
	REGISTER_FUNC(l,maAttributeEnd)
	REGISTER_FUNC(l,maTransformBegin)
	REGISTER_FUNC(l,maTransformEnd)
	REGISTER_FUNC(l,maReverseOrientation)
	REGISTER_FUNC(l,maObjectEnd)
	REGISTER_FUNC(l,maObjectBegin)
	REGISTER_FUNC(l,maObjectInstance)
	REGISTER_FUNC(l,maTexture)
	REGISTER_FUNC(l,maWorldEnd)
	REGISTER_FUNC(l,maFrameEnd)


	/////////////////////////////////
	REGISTER_FUNC(l,maBeginParam)
	REGISTER_FUNC(l,maAddPointParam)
	REGISTER_FUNC(l,maAddSpectrumParam)
	REGISTER_FUNC(l,maAddFloatArrayParam)
	REGISTER_FUNC(l,maAddIntArrayParam)
	REGISTER_FUNC(l,maAddUnsignedArrayParam)
	REGISTER_FUNC(l,maAddPointArrayParam)
	REGISTER_FUNC(l,maEndParam)

}



