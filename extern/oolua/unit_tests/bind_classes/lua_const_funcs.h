#ifndef LUA_CONST_FUNCS_H_
#	define LUA_CONST_FUNCS_H_

#	include "cpp_const_funcs.h"
#	include "oolua.h"

//OOLUA_CLASS_NO_BASES(C_simple)
LUA_PROXY_CLASS(C_simple)
OOLUA_NO_TYPEDEFS
LUA_CTORS()
LUA_MEM_FUNC(void(int const&),set_int)
LUA_MEM_FUNC_CONST(int(),get_int)
LUA_MEM_FUNC(void(),func)
LUA_MEM_FUNC_CONST(C_simple const(),bar)
LUA_MEM_FUNC_CONST(C_simple const*(),bar1)
LUA_PROXY_CLASS_END

#endif
