#ifndef EXPOSE_PULLS_STUB_PARAM_H_
#	define EXPOSE_PULLS_STUB_PARAM_H_

#	include "oolua.h"
#	include "cpp_pulls_stub_param.h"

LUA_PROXY_CLASS(Stub)
	OOLUA_NO_TYPEDEFS
LUA_PROXY_CLASS_END

LUA_PROXY_CLASS(Pulls_stub)
	OOLUA_TYPEDEFS Abstract OOLUA_END_TYPES
	LUA_MEM_FUNC(void(Stub&),ref)
	LUA_MEM_FUNC(void(Stub const&),ref_const)
	LUA_MEM_FUNC(void(Stub*),ptr)
	LUA_MEM_FUNC(void(Stub const*),ptr_const)
#ifndef _MSC_VER
	LUA_MEM_FUNC(void(Stub const* const),const_ptr_const)
#endif
	LUA_MEM_FUNC(void(Stub const*&),ref_ptr_const)
	LUA_MEM_FUNC(void(Stub const* const&),ref_const_ptr_const)
LUA_PROXY_CLASS_END

#endif

