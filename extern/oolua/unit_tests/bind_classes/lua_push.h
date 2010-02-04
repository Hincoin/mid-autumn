#ifndef LUA_PUSH_H_
#	define LUA_PUSH_H_

#	include "oolua.h"
#	include "cpp_push.h"


LUA_PROXY_CLASS(Push)
OOLUA_NO_TYPEDEFS
LUA_CTORS()
LUA_MEM_FUNC(Push&(),ref)
LUA_MEM_FUNC(Push const&(),ref_const )
LUA_MEM_FUNC(Push*(),ptr)
LUA_MEM_FUNC(Push const*(),ptr_const)
LUA_MEM_FUNC(Push const* (),const_ptr_const)
LUA_MEM_FUNC_CONST(void(),const_func)
LUA_MEM_FUNC(int(),int_value)
LUA_MEM_FUNC(int&(),int_ref)
LUA_MEM_FUNC(int*(),int_ptr)
LUA_MEM_FUNC(int const&(),int_ref_const)
LUA_MEM_FUNC(int const* const&(),int_ref_const_ptr_const)
LUA_MEM_FUNC(int const*&(),int_ref_ptr_const)
OOLUA_PUBLIC_MEMBER_GET_SET(i_)
LUA_PROXY_CLASS_END



#endif//LUA_PUSH_H_
