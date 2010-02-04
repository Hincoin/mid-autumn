///////////////////////////////////////////////////////////////////////////////
///  @file lua_class_ops.h
///  <TODO: insert file description here>
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence
///  This work is licenced under a Creative Commons Licence. \n
///  see: \n
///  http://creativecommons.org/licenses/by-nc-sa/3.0/ \n
///  and: \n
///  http://creativecommons.org/licenses/by-nc-sa/3.0/legalcode \n
///  For more details.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUA_CLASS_OPS_H_
#	define LUA_CLASS_OPS_H_

//include the cpp class header
#include "cpp_class_ops.h"
//and oolua
#include "oolua.h"


LUA_PROXY_CLASS(Class_ops)
	OOLUA_TYPEDEFS
		OOLUA::Equal_op,
		OOLUA::Less_op,
		OOLUA::Less_equal_op,
		OOLUA::Add_op,
		OOLUA::Sub_op,
		OOLUA::Mul_op,
		OOLUA::Div_op
	OOLUA_END_TYPES
	LUA_CTORS()
LUA_MEM_FUNC_CONST(int const &(),geti)
LUA_MEM_FUNC(void(int const&),seti)
LUA_MEM_FUNC_CONST(Class_ops const*(),pointer_to_const)
LUA_PROXY_CLASS_END


#endif// LUA_CLASS_OPS_H_
