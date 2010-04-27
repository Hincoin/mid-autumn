#include <iostream>
#include "script.hpp"
#include "oolua_error.h"

#include "ptr_var.hpp"

int main(int argc,char** argv)
{
	OOLUA::Script lua ;
	register_api(lua);
	register_all_class(lua);
	for(int i = 1;i < argc;++i)
	{
		if(!(lua.run_file(argv[i]) && lua.call("main"))) 
		{
			printf("Failed to run file:%s \n",argv[i]);
			OOLUA::lua_stack_dump(lua);
		}
	}
	if(argc == 1 && ! (lua.run_file("main.lua") && lua.call("main") ))
	{
		printf("Failed to run file:%s \n","main.lua");
		OOLUA::lua_stack_dump(lua);
	}

	return 0;
}


