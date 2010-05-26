#include <iostream>
#include "ptr_var.hpp"

/*
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
	if(argc == 1 && ! (lua.run_file("main.lua") && lua.call("main",0,1,0,1) ))
	{
		printf("Failed to run file:%s \n","main.lua");
		OOLUA::lua_stack_dump(lua);
	}

	return 0;
}
*/
/*
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>


#include "render_client.hpp"
using namespace ma;
using namespace net;
using namespace rpc;
int main(int argc,char* argv[])
{
	OOLUA::Script lua ;
	register_api(lua);
	register_all_class(lua);
	
	try
	{
		if(argc != 4)
		{
			std::cerr << "Usage: pbr_ext <script> <host> <port>"<<std::endl;
			return 1;
		}
		lua.run_file(argv[1]);

		boost::asio::io_service io_service;
		render_client c(lua,io_service,argv[2],argv[3]);
		io_service.run();
	}
	catch(std::exception& e)
	{}
	return 0;
}

*/
