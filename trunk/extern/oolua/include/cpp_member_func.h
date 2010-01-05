///////////////////////////////////////////////////////////////////////////////
///  @file cpp_member_func.h
///  @remarks Warning this file was generated, edits to it will not persist if 
///  the file is regenerated.
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence 
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////
#ifndef CPP_MEMBER_FUNC_H_
#	define CPP_MEMBER_FUNC_H_

#	include "param_traits.h"
#ifdef _MSC_VER 
#	define MSC_PUSH_DISABLE_CONDTIONAL_CONSTANT \
	__pragma(warning(push)) \
	__pragma(warning(disable : 4127)) 
#	define MSC_POP_COMPILER_WARNING \
	__pragma(warning(pop)) 
#else
#	define MSC_PUSH_DISABLE_CONDTIONAL_CONSTANT 
#	define MSC_POP_COMPILER_WARNING 
#endif
//#define OOLUA_CONST_FUNC const
//#define OOLUA_NON_CONST_FUNC
//
////member function macros
//#define OOLUA_MEM_FUNC_0(return_value,func)\
//	LUA_CLASS_MEMBER_FUNCTION_0(func,return_value,func,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_0_CONST(return_value,func)\
//	LUA_CLASS_MEMBER_FUNCTION_0(func,return_value,func,OOLUA_CONST_FUNC)
//#define OOLUA_MEM_FUNC_0_RENAME(name,return_value,func)\
//	LUA_CLASS_MEMBER_FUNCTION_0(name,return_value,func,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_0_CONST_RENAME(name,return_value,func)\
//	LUA_CLASS_MEMBER_FUNCTION_0(name,return_value,func,OOLUA_CONST_FUNC)
//
//
//#define OOLUA_MEM_FUNC_1(return_value,func,P1)\
//	LUA_CLASS_MEMBER_FUNCTION_1(func,return_value,func,P1,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_1_CONST(return_value,func,P1)\
//	LUA_CLASS_MEMBER_FUNCTION_1(func,return_value,func,P1,OOLUA_CONST_FUNC)
//#define OOLUA_MEM_FUNC_1_RENAME(name,return_value,func,P1)\
//	LUA_CLASS_MEMBER_FUNCTION_1(name,return_value,func,P1,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_1_CONST_RENAME(name,return_value,func,P1)\
//	LUA_CLASS_MEMBER_FUNCTION_1(name,return_value,func,P1,OOLUA_CONST_FUNC)
//
//
//#define OOLUA_MEM_FUNC_2(return_value,func,P1,P2)\
//	LUA_CLASS_MEMBER_FUNCTION_2(func,return_value,func,P1,P2,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_2_CONST(return_value,func,P1,P2)\
//	LUA_CLASS_MEMBER_FUNCTION_2(func,return_value,func,P1,P2,OOLUA_CONST_FUNC)
//#define OOLUA_MEM_FUNC_2_RENAME(name,return_value,func,P1,P2)\
//	LUA_CLASS_MEMBER_FUNCTION_2(name,return_value,func,P1,P2,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_2_CONST_RENAME(name,return_value,func,P1,P2)\
//	LUA_CLASS_MEMBER_FUNCTION_2(name,return_value,func,P1,P2,OOLUA_CONST_FUNC)
//
//
//#define OOLUA_MEM_FUNC_3(return_value,func,P1,P2,P3)\
//	LUA_CLASS_MEMBER_FUNCTION_3(func,return_value,func,P1,P2,P3,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_3_CONST(return_value,func,P1,P2,P3)\
//	LUA_CLASS_MEMBER_FUNCTION_3(func,return_value,func,P1,P2,P3,OOLUA_CONST_FUNC)
//#define OOLUA_MEM_FUNC_3_RENAME(name,return_value,func,P1,P2,P3)\
//	LUA_CLASS_MEMBER_FUNCTION_3(name,return_value,func,P1,P2,P3,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_3_CONST_RENAME(name,return_value,func,P1,P2,P3)\
//	LUA_CLASS_MEMBER_FUNCTION_3(name,return_value,func,P1,P2,P3,OOLUA_CONST_FUNC)
//
//
//#define OOLUA_MEM_FUNC_4(return_value,func,P1,P2,P3,P4)\
//	LUA_CLASS_MEMBER_FUNCTION_4(func,return_value,func,P1,P2,P3,P4,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_4_CONST(return_value,func,P1,P2,P3,P4)\
//	LUA_CLASS_MEMBER_FUNCTION_4(func,return_value,func,P1,P2,P3,P4,OOLUA_CONST_FUNC)
//#define OOLUA_MEM_FUNC_4_RENAME(name,return_value,func,P1,P2,P3,P4)\
//	LUA_CLASS_MEMBER_FUNCTION_4(name,return_value,func,P1,P2,P3,P4,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_4_CONST_RENAME(name,return_value,func,P1,P2,P3,P4)\
//	LUA_CLASS_MEMBER_FUNCTION_4(name,return_value,func,P1,P2,P3,P4,OOLUA_CONST_FUNC)
//
//
//#define OOLUA_MEM_FUNC_5(return_value,func,P1,P2,P3,P4,P5)\
//	LUA_CLASS_MEMBER_FUNCTION_5(func,return_value,func,P1,P2,P3,P4,P5,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_5_CONST(return_value,func,P1,P2,P3,P4,P5)\
//	LUA_CLASS_MEMBER_FUNCTION_5(func,return_value,func,P1,P2,P3,P4,P5,OOLUA_CONST_FUNC)
//#define OOLUA_MEM_FUNC_5_RENAME(name,return_value,func,P1,P2,P3,P4,P5)\
//	LUA_CLASS_MEMBER_FUNCTION_5(name,return_value,func,P1,P2,P3,P4,P5,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_5_CONST_RENAME(name,return_value,func,P1,P2,P3,P4,P5)\
//	LUA_CLASS_MEMBER_FUNCTION_5(name,return_value,func,P1,P2,P3,P4,P5,OOLUA_CONST_FUNC)
//
//
//#define OOLUA_MEM_FUNC_6(return_value,func,P1,P2,P3,P4,P5,P6)\
//	LUA_CLASS_MEMBER_FUNCTION_6(func,return_value,func,P1,P2,P3,P4,P5,P6,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_6_CONST(return_value,func,P1,P2,P3,P4,P5,P6)\
//	LUA_CLASS_MEMBER_FUNCTION_6(func,return_value,func,P1,P2,P3,P4,P5,P6,OOLUA_CONST_FUNC)
//#define OOLUA_MEM_FUNC_6_RENAME(name,return_value,func,P1,P2,P3,P4,P5,P6)\
//	LUA_CLASS_MEMBER_FUNCTION_6(name,return_value,func,P1,P2,P3,P4,P5,P6,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_6_CONST_RENAME(name,return_value,func,P1,P2,P3,P4,P5,P6)\
//	LUA_CLASS_MEMBER_FUNCTION_6(name,return_value,func,P1,P2,P3,P4,P5,P6,OOLUA_CONST_FUNC)
//
//
//#define OOLUA_MEM_FUNC_7(return_value,func,P1,P2,P3,P4,P5,P6,P7)\
//	LUA_CLASS_MEMBER_FUNCTION_7(func,return_value,func,P1,P2,P3,P4,P5,P6,P7,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_7_CONST(return_value,func,P1,P2,P3,P4,P5,P6,P7)\
//	LUA_CLASS_MEMBER_FUNCTION_7(func,return_value,func,P1,P2,P3,P4,P5,P6,P7,OOLUA_CONST_FUNC)
//#define OOLUA_MEM_FUNC_7_RENAME(name,return_value,func,P1,P2,P3,P4,P5,P6,P7)\
//	LUA_CLASS_MEMBER_FUNCTION_7(name,return_value,func,P1,P2,P3,P4,P5,P6,P7,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_7_CONST_RENAME(name,return_value,func,P1,P2,P3,P4,P5,P6,P7)\
//	LUA_CLASS_MEMBER_FUNCTION_7(name,return_value,func,P1,P2,P3,P4,P5,P6,P7,OOLUA_CONST_FUNC)
//
//
//#define OOLUA_MEM_FUNC_8(return_value,func,P1,P2,P3,P4,P5,P6,P7,P8)\
//	LUA_CLASS_MEMBER_FUNCTION_8(func,return_value,func,P1,P2,P3,P4,P5,P6,P7,P8,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_8_CONST(return_value,func,P1,P2,P3,P4,P5,P6,P7,P8)\
//	LUA_CLASS_MEMBER_FUNCTION_8(func,return_value,func,P1,P2,P3,P4,P5,P6,P7,P8,OOLUA_CONST_FUNC)
//#define OOLUA_MEM_FUNC_8_RENAME(name,return_value,func,P1,P2,P3,P4,P5,P6,P7,P8)\
//	LUA_CLASS_MEMBER_FUNCTION_8(name,return_value,func,P1,P2,P3,P4,P5,P6,P7,P8,OOLUA_NON_CONST_FUNC)
//#define OOLUA_MEM_FUNC_8_CONST_RENAME(name,return_value,func,P1,P2,P3,P4,P5,P6,P7,P8)\
//	LUA_CLASS_MEMBER_FUNCTION_8(name,return_value,func,P1,P2,P3,P4,P5,P6,P7,P8,OOLUA_CONST_FUNC)
//
//
////param return macros
//#define OOLUA_BACK_INTERNAL_(NUM)\
//MSC_PUSH_DISABLE_CONDTIONAL_CONSTANT \
//	if( P ## NUM ## _::out )\
//		OOLUA::Member_func_helper<P ## NUM ##_,P ## NUM ##_::owner>::push2lua(l,p ## NUM);\
//MSC_POP_COMPILER_WARNING
//
//
//#define OOLUA_BACK_INTERNAL_1\
//	OOLUA_BACK_INTERNAL_(1);
//
//#define OOLUA_BACK_INTERNAL_2\
//	OOLUA_BACK_INTERNAL_1\
//	OOLUA_BACK_INTERNAL_(2);
//
//#define OOLUA_BACK_INTERNAL_3\
//	OOLUA_BACK_INTERNAL_2\
//	OOLUA_BACK_INTERNAL_(3);
//
//#define OOLUA_BACK_INTERNAL_4\
//	OOLUA_BACK_INTERNAL_3\
//	OOLUA_BACK_INTERNAL_(4);
//
//#define OOLUA_BACK_INTERNAL_5\
//	OOLUA_BACK_INTERNAL_4\
//	OOLUA_BACK_INTERNAL_(5);
//
//#define OOLUA_BACK_INTERNAL_6\
//	OOLUA_BACK_INTERNAL_5\
//	OOLUA_BACK_INTERNAL_(6);
//
//#define OOLUA_BACK_INTERNAL_7\
//	OOLUA_BACK_INTERNAL_6\
//	OOLUA_BACK_INTERNAL_(7);
//
//#define OOLUA_BACK_INTERNAL_8\
//	OOLUA_BACK_INTERNAL_7\
//	OOLUA_BACK_INTERNAL_(8);
//
////param macros
//#define OOLUA_INTERNAL_PARAM(NUM,PARAM)\
//	typedef param_type<PARAM > P ## NUM ##_;\
//	static P ## NUM ##_::pull_type p ## NUM;\
//	MSC_PUSH_DISABLE_CONDTIONAL_CONSTANT\
//	if( P ## NUM ##_::in )\
//		OOLUA::Member_func_helper<P ## NUM ##_,P ## NUM ##_::owner>::pull2cpp(l,p ## NUM);\
//	MSC_POP_COMPILER_WARNING
//
//#define OOLUA_PARAMS_INTERNAL_1(PARAM1)\
//	OOLUA_INTERNAL_PARAM(1,PARAM1)
//
//#define OOLUA_PARAMS_INTERNAL_2(PARAM1,PARAM2)\
//	OOLUA_INTERNAL_PARAM(2,PARAM2)\
//	OOLUA_PARAMS_INTERNAL_1(PARAM1)
//
//#define OOLUA_PARAMS_INTERNAL_3(PARAM1,PARAM2,PARAM3)\
//	OOLUA_INTERNAL_PARAM(3,PARAM3)\
//	OOLUA_PARAMS_INTERNAL_2(PARAM1,PARAM2)
//
//#define OOLUA_PARAMS_INTERNAL_4(PARAM1,PARAM2,PARAM3,PARAM4)\
//	OOLUA_INTERNAL_PARAM(4,PARAM4)\
//	OOLUA_PARAMS_INTERNAL_3(PARAM1,PARAM2,PARAM3)
//
//#define OOLUA_PARAMS_INTERNAL_5(PARAM1,PARAM2,PARAM3,PARAM4,PARAM5)\
//	OOLUA_INTERNAL_PARAM(5,PARAM5)\
//	OOLUA_PARAMS_INTERNAL_4(PARAM1,PARAM2,PARAM3,PARAM4)
//
//#define OOLUA_PARAMS_INTERNAL_6(PARAM1,PARAM2,PARAM3,PARAM4,PARAM5,PARAM6)\
//	OOLUA_INTERNAL_PARAM(6,PARAM6)\
//	OOLUA_PARAMS_INTERNAL_5(PARAM1,PARAM2,PARAM3,PARAM4,PARAM5)
//
//#define OOLUA_PARAMS_INTERNAL_7(PARAM1,PARAM2,PARAM3,PARAM4,PARAM5,PARAM6,PARAM7)\
//	OOLUA_INTERNAL_PARAM(7,PARAM7)\
//	OOLUA_PARAMS_INTERNAL_6(PARAM1,PARAM2,PARAM3,PARAM4,PARAM5,PARAM6)
//
//#define OOLUA_PARAMS_INTERNAL_8(PARAM1,PARAM2,PARAM3,PARAM4,PARAM5,PARAM6,PARAM7,PARAM8)\
//	OOLUA_INTERNAL_PARAM(8,PARAM8)\
//	OOLUA_PARAMS_INTERNAL_7(PARAM1,PARAM2,PARAM3,PARAM4,PARAM5,PARAM6,PARAM7)
//
////proxy implementations
//#define LUA_CLASS_MEMBER_FUNCTION_0(func_name,return_value,func,mod)\
//int func_name(lua_State* const l)mod\
//{\
//	assert(m_this);\
//	typedef param_type<return_value > R;\
//	static R::type (class_::*f )()mod  = &class_::func;\
//	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call(l,m_this,f);\
//	return total_out_params< Type_list<out_p<return_value > >::type> ::out;\
//}
//#define LUA_CLASS_MEMBER_FUNCTION_1(func_name,return_value,func,P1,mod)\
//int func_name(lua_State* const l)mod\
//{\
//	assert(m_this);\
//	OOLUA_PARAMS_INTERNAL_1(P1)\
//	typedef param_type<return_value > R;\
//	static R::type (class_::*f )(P1_::type)mod  = &class_::func;\
//	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_>(l,m_this,f,p1);\
//	OOLUA_BACK_INTERNAL_1\
//	return total_out_params< Type_list<out_p<return_value >,P1_ >::type> ::out;\
//}
//#define LUA_CLASS_MEMBER_FUNCTION_2(func_name,return_value,func,P1,P2,mod)\
//int func_name(lua_State* const l)mod\
//{\
//	assert(m_this);\
//	OOLUA_PARAMS_INTERNAL_2(P1,P2)\
//	typedef param_type<return_value > R;\
//	static R::type (class_::*f )(P1_::type,P2_::type)mod  = &class_::func;\
//	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_>(l,m_this,f,p1,p2);\
//	OOLUA_BACK_INTERNAL_2\
//	return total_out_params< Type_list<out_p<return_value >,P1_,P2_ >::type> ::out;\
//}
//#define LUA_CLASS_MEMBER_FUNCTION_3(func_name,return_value,func,P1,P2,P3,mod)\
//int func_name(lua_State* const l)mod\
//{\
//	assert(m_this);\
//	OOLUA_PARAMS_INTERNAL_3(P1,P2,P3)\
//	typedef param_type<return_value > R;\
//	static R::type (class_::*f )(P1_::type,P2_::type,P3_::type)mod  = &class_::func;\
//	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_>(l,m_this,f,p1,p2,p3);\
//	OOLUA_BACK_INTERNAL_3\
//	return total_out_params< Type_list<out_p<return_value >,P1_,P2_,P3_ >::type> ::out;\
//}
//#define LUA_CLASS_MEMBER_FUNCTION_4(func_name,return_value,func,P1,P2,P3,P4,mod)\
//int func_name(lua_State* const l)mod\
//{\
//	assert(m_this);\
//	OOLUA_PARAMS_INTERNAL_4(P1,P2,P3,P4)\
//	typedef param_type<return_value > R;\
//	static R::type (class_::*f )(P1_::type,P2_::type,P3_::type,P4_::type)mod  = &class_::func;\
//	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,P4_>(l,m_this,f,p1,p2,p3,p4);\
//	OOLUA_BACK_INTERNAL_4\
//	return total_out_params< Type_list<out_p<return_value >,P1_,P2_,P3_,P4_ >::type> ::out;\
//}
//#define LUA_CLASS_MEMBER_FUNCTION_5(func_name,return_value,func,P1,P2,P3,P4,P5,mod)\
//int func_name(lua_State* const l)mod\
//{\
//	assert(m_this);\
//	OOLUA_PARAMS_INTERNAL_5(P1,P2,P3,P4,P5)\
//	typedef param_type<return_value > R;\
//	static R::type (class_::*f )(P1_::type,P2_::type,P3_::type,P4_::type,P5_::type)mod  = &class_::func;\
//	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,P4_,P5_>(l,m_this,f,p1,p2,p3,p4,p5);\
//	OOLUA_BACK_INTERNAL_5\
//	return total_out_params< Type_list<out_p<return_value >,P1_,P2_,P3_,P4_,P5_ >::type> ::out;\
//}
//#define LUA_CLASS_MEMBER_FUNCTION_6(func_name,return_value,func,P1,P2,P3,P4,P5,P6,mod)\
//int func_name(lua_State* const l)mod\
//{\
//	assert(m_this);\
//	OOLUA_PARAMS_INTERNAL_6(P1,P2,P3,P4,P5,P6)\
//	typedef param_type<return_value > R;\
//	static R::type (class_::*f )(P1_::type,P2_::type,P3_::type,P4_::type,P5_::type,P6_::type)mod  = &class_::func;\
//	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,P4_,P5_,P6_>(l,m_this,f,p1,p2,p3,p4,p5,p6);\
//	OOLUA_BACK_INTERNAL_6\
//	return total_out_params< Type_list<out_p<return_value >,P1_,P2_,P3_,P4_,P5_,P6_ >::type> ::out;\
//}
//#define LUA_CLASS_MEMBER_FUNCTION_7(func_name,return_value,func,P1,P2,P3,P4,P5,P6,P7,mod)\
//int func_name(lua_State* const l)mod\
//{\
//	assert(m_this);\
//	OOLUA_PARAMS_INTERNAL_7(P1,P2,P3,P4,P5,P6,P7)\
//	typedef param_type<return_value > R;\
//	static R::type (class_::*f )(P1_::type,P2_::type,P3_::type,P4_::type,P5_::type,P6_::type,P7_::type)mod  = &class_::func;\
//	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,P4_,P5_,P6_,P7_>(l,m_this,f,p1,p2,p3,p4,p5,p6,p7);\
//	OOLUA_BACK_INTERNAL_7\
//	return total_out_params< Type_list<out_p<return_value >,P1_,P2_,P3_,P4_,P5_,P6_,P7_ >::type> ::out;\
//}
//#define LUA_CLASS_MEMBER_FUNCTION_8(func_name,return_value,func,P1,P2,P3,P4,P5,P6,P7,P8,mod)\
//int func_name(lua_State* const l)mod\
//{\
//	assert(m_this);\
//	OOLUA_PARAMS_INTERNAL_8(P1,P2,P3,P4,P5,P6,P7,P8)\
//	typedef param_type<return_value > R;\
//	static R::type (class_::*f )(P1_::type,P2_::type,P3_::type,P4_::type,P5_::type,P6_::type,P7_::type,P8_::type)mod  = &class_::func;\
//	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,P4_,P5_,P6_,P7_,P8_>(l,m_this,f,p1,p2,p3,p4,p5,p6,p7,p8);\
//	OOLUA_BACK_INTERNAL_8\
//	return total_out_params< Type_list<out_p<return_value >,P1_,P2_,P3_,P4_,P5_,P6_,P7_,P8_ >::type> ::out;\
//}
//

#include <boost/type_traits.hpp>
#include "boost/preprocessor/cat.hpp"
#include "boost/preprocessor/enum.hpp"
#include "boost/preprocessor/enum_params.hpp"
#include "boost/preprocessor/enum_shifted_params.hpp"
#include "boost/preprocessor/repeat.hpp"
#include "boost/mpl/accumulate.hpp"
#include "boost/mpl/int.hpp"
#include "boost/mpl/transform.hpp"
#include "boost/mpl/vector.hpp"
#include "boost/mpl/at.hpp"
#include "boost/mpl/list.hpp"
#include <boost/fusion/container.hpp>

//#include "boost/mpl/accumulate.hpp"
//#include "boost/mpl/accumulate.hpp"
//#include "boost/mpl/accumulate.hpp"
//#include "boost/mpl/accumulate.hpp"

#include "boost/mpl/limits/list.hpp"
#include "boost/fusion/adapted.hpp"
#include "boost/fusion/sequence/intrinsic.hpp"
#include "member_func_helper.h"
//
//#define LUA_CLASS_MEMBER_FUNCTION_2(func_name,return_value,func,P1,P2,mod)\
//	int func_name(lua_State* const l)mod\
//{\
//	assert(m_this);\
//	OOLUA_PARAMS_INTERNAL_2(P1,P2)\
//	typedef param_type<return_value > R;\
//	static R::type (class_::*f )(P1_::type,P2_::type)mod  = &class_::func;\
//	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_>(l,m_this,f,p1,p2);\
//	OOLUA_BACK_INTERNAL_2\
//	return total_out_params< Type_list<out_p<return_value >,P1_,P2_ >::type> ::out;\
//}

#define LUA_CLASS_MEM_FUNC_RENAME(func_type,func_rename,func_name,mod)\
int func_rename(lua_State* const l)mod\
{\
	enum{func_is_ = 0,func_is_const=1};\
	assert(m_this);\
	typedef boost::function_traits<func_type>::result_type result_type;\
	typedef param_type<result_type> R;\
	typedef func_param_type_list<func_type>::type parameter_list;\
	{\
	internal_param_pull2_cpp_push2_lua<parameter_list> scope_value(l);\
	static add_member_func_pointer<class_,func_type,1 == BOOST_PP_CAT(func_is_ , mod)>::type f = &class_::func_name;\
	OOLUA::Proxy_caller<R,class_>::call<parameter_list>(l,m_this,f,scope_value.v);\
	}\
	typedef boost::mpl::transform<parameter_list,to_param_type<boost::mpl::_1> >::type params_list;\
	return out_params_count< boost::mpl::push_front<\
	params_list,\
	out_p<result_type > \
	>::type >::value;\
}\

#define LUA_CLASS_MEM_FUNC(func_type,func_name,mod)\
	LUA_CLASS_MEM_FUNC_RENAME(func_type,func_name,func_name,mod)\

#define LUA_MEM_FUNC(func_type,func_name)\
	LUA_CLASS_MEM_FUNC(func_type,func_name,)\

#define LUA_MEM_FUNC_CONST(func_type,func_name)\
	LUA_CLASS_MEM_FUNC(func_type,func_name,const)

#define LUA_MEM_FUNC_RENAME(func_type,func_rename,func_name)\
	LUA_CLASS_MEM_FUNC_RENAME(func_type,func_rename,func_name,)\


#define LUA_MEM_FUNC_RENAME_CONST(func_type,func_rename,func_name)\
	LUA_CLASS_MEM_FUNC_RENAME(func_type,func_rename,func_name,const)\

namespace OOLUA{
	template<typename I,typename T>
	struct out_params_add{
		typedef boost::mpl::int_<I::value + T::out> type;
	};
	template<typename Seq>
	struct out_params_count{
		typedef typename 
			boost::mpl::accumulate<Seq,boost::mpl::int_<0>,
			out_params_add<boost::mpl::_1,boost::mpl::_2> >::type result_type;
		enum{value = result_type::value};
	};
	template<class C,typename FuncT,int arity,bool is_const>
	struct add_member_func_pointer_type_impl;
	template<typename FuncT,int arity>
	struct func_parameter_type_list_impl;

#   define MA_PP_FUNC_ARGS_AUX(z, N, FuncT) \
	typedef typename param_type<typename boost::function_traits<FuncT>::BOOST_PP_CAT(BOOST_PP_CAT(arg,BOOST_PP_INC(N)), _type)>::type  arg_type##N; \

#define  MA_PP_FUNC_PARAM_ARGS_AUX(z,N,FuncT)\
	typedef typename boost::function_traits<FuncT>::BOOST_PP_CAT(BOOST_PP_CAT(arg,BOOST_PP_INC(N)), _type)  arg_type##N; \

#define MA_PP_ADD_MEMBER_FUNC_POINTER(z,N,_)\
	template<typename FuncT>\
	struct func_parameter_type_list_impl<FuncT,N>{\
	BOOST_STATIC_ASSERT((N == boost::function_traits<FuncT>::arity));\
	BOOST_PP_REPEAT( \
	N \
	, MA_PP_FUNC_PARAM_ARGS_AUX \
	, FuncT \
	) \
	typedef boost::mpl::list<BOOST_PP_ENUM_PARAMS(N,arg_type)> type;\
	};\
	template<class C,typename FuncT>\
	struct add_member_func_pointer_type_impl<C,FuncT,N,false>\
	{\
	BOOST_STATIC_ASSERT((N == boost::function_traits<FuncT>::arity));\
	BOOST_PP_REPEAT( \
	N \
	, MA_PP_FUNC_ARGS_AUX \
	, FuncT \
	) \
	typedef typename param_type<typename boost::function_traits<FuncT>::result_type>::type result_type;\
	typedef result_type (C::* type)(BOOST_PP_ENUM_PARAMS(N,arg_type));\
	};\
	template<class C,typename FuncT>\
	struct add_member_func_pointer_type_impl<C,FuncT,N,true>\
	{\
	BOOST_STATIC_ASSERT((N == boost::function_traits<FuncT>::arity));\
	BOOST_PP_REPEAT( \
	N \
	, MA_PP_FUNC_ARGS_AUX \
	, FuncT \
	) \
	typedef typename param_type<typename boost::function_traits<FuncT>::result_type>::type result_type;\
	typedef result_type (C::* type)(BOOST_PP_ENUM_PARAMS(N,arg_type))const;\
	};\

#ifndef MA_FUNCTION_MAX_ARG_NUM
#define MA_FUNCTION_MAX_ARG_NUM 10
#endif

	BOOST_PP_REPEAT(MA_FUNCTION_MAX_ARG_NUM,MA_PP_ADD_MEMBER_FUNC_POINTER,_)

#undef MA_PP_ADD_MEMBER_FUNC_POINTER
#undef MA_PP_FUNC_ARGS_AUX

	template<class C,typename FuncT,bool is_const>
	struct add_member_func_pointer:add_member_func_pointer_type_impl<C,FuncT,boost::function_traits<FuncT>::arity,is_const>{
	};
	template<typename FuncT>
	struct func_param_type_list:func_parameter_type_list_impl<FuncT,boost::function_traits<FuncT>::arity>
	{};
	template<typename T>
	struct to_pull_type{
		typedef param_type<T> parameter_type;
		typedef typename parameter_type::pull_type type;
	};
	template<typename T>
	struct to_param_type{
		typedef param_type<T> type;
	};
	template<typename TypeSeq>
	struct internal_param_pull2_cpp_push2_lua{
		typedef typename boost::mpl::transform<TypeSeq,to_param_type<boost::mpl::_1> >::type param_type_list;
		typedef typename boost::mpl::transform<TypeSeq,to_pull_type<boost::mpl::_1> >::type pull_type_list;
		typedef typename  boost::fusion::result_of::as_vector<pull_type_list>::type vector_type;
		/*static*/ vector_type v;
		lua_State* const l;

		template<int idx>
		struct pull2cpp{
			void operator()(vector_type& v,lua_State* const l)const
			{
				BOOST_STATIC_ASSERT((!boost::mpl::empty<TypeSeq>::value));
				typedef typename boost::mpl::at_c<TypeSeq,boost::mpl::size<TypeSeq>::value - 1 - idx>::type cur_type;
				typedef param_type<cur_type> parameter_type;
				//typedef cur_type parameter_type;
				MSC_PUSH_DISABLE_CONDTIONAL_CONSTANT
				if(parameter_type :: in)
					OOLUA::Member_func_helper<parameter_type,parameter_type::owner>::pull2cpp(l,boost::fusion::at_c<boost::mpl::size<TypeSeq>::value - 1 - idx>(v));
				MSC_POP_COMPILER_WARNING
			}
		};
		template<int idx>
		struct push2lua{
			void operator()(vector_type& v,lua_State* const l)const
			{
				typedef typename boost::mpl::at_c<TypeSeq,idx>::type cur_type;
				typedef param_type<cur_type> parameter_type;
				//typedef cur_type parameter_type;
				MSC_PUSH_DISABLE_CONDTIONAL_CONSTANT
				if (parameter_type::out)
					OOLUA::Member_func_helper<parameter_type,parameter_type::owner>::push2lua(l,boost::fusion::at_c<idx>(v));
				MSC_POP_COMPILER_WARNING
			}
		};
		template<int b,int e,template<int> class F>
		struct for_each{
			BOOST_STATIC_ASSERT(b<e);
			static void apply(vector_type& v,lua_State* const l)
			{
				F<b>()(v,l);
				for_each<b+1,e,F>::apply(v,l);
			}
		};
		template<int e,template<int> class F>
		struct for_each<e,e,F>{ static void apply(vector_type&,lua_State* const){}};

		internal_param_pull2_cpp_push2_lua(lua_State* const ll):l(ll)
		{
			for_each<0,boost::mpl::size<param_type_list>::value,pull2cpp>::apply(v,l);
		}
		~internal_param_pull2_cpp_push2_lua(){
			for_each<0,boost::mpl::size<param_type_list>::value,push2lua>::apply(v,l);
		}
	private:
		internal_param_pull2_cpp_push2_lua& operator=(const internal_param_pull2_cpp_push2_lua&);
	};
}


#endif 
