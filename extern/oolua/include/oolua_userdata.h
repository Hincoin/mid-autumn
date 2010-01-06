#ifndef OOLUA_USERDATA_H_
#	define OOLUA_USERDATA_H_

#include <cstring>
#include "lua_ref.h"
namespace OOLUA
{
    namespace INTERNAL
    {
        struct Lua_ud
        {
			void* void_class_ptr;
			char* name;
			char* none_const_name;//none constant name of the class
			int name_size;//size of name
            bool gc;//should it be garbage collected
			//Lua_table_ref mt;//meta_table
			Lua_ud_ref ud;//reference to self;
			void release(){ud.~Lua_ud_ref();}
        };
		inline bool id_is_const(Lua_ud* ud)
		{
			return ud->name != ud->none_const_name;
		}
#ifdef UNSAFE_ID_COMPARE
		inline bool ids_equal(char* lhsName,int /*lhsSize*/,char* rhsName,int /*rhsSize*/)
		{
			return lhsName == rhsName;
		}
#else
		inline bool ids_equal(char* lhsName,int lhsSize,char* rhsName,int rhsSize)
		{
			if(lhsSize != rhsSize)return false;
			return memcmp(lhsName,rhsName,lhsSize) == 0;
		}
#endif
    }
}
#endif
