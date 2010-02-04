///////////////////////////////////////////////////////////////////////////////
///  @file lua_ownership.h
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
#ifndef LUA_OWNERSHIP_H_
#	define LUA_OWNERSHIP_H_

//include the cpp class header
#include "cpp_ownership.h"
//and oolua
#include "oolua.h"

LUA_PROXY_CLASS(A_simple)
OOLUA_NO_TYPEDEFS
LUA_CTORS()
LUA_PROXY_CLASS_END

LUA_PROXY_CLASS(A_derived,A_simple)
OOLUA_NO_TYPEDEFS
LUA_CTORS()
LUA_PROXY_CLASS_END

LUA_PROXY_CLASS(B_simple)
OOLUA_NO_TYPEDEFS
LUA_CTORS()
LUA_MEM_FUNC(void(cpp_acquire_ptr<A_simple*>),add_A_ptr)
LUA_MEM_FUNC(lua_acquire_ptr<A_simple*>(),create_A)
LUA_MEM_FUNC(B_simple*(),return_this)
LUA_MEM_FUNC(B_simple const*(),return_this_const)
LUA_PROXY_CLASS_END


#endif// LUA_OWNERSHIP_H_
