#include <iostream>
#include "ptr_var.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>

#include "render_client.hpp"
#include "net_ext.hpp"
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
using namespace ma;
using namespace net;
using namespace rpc;
int main(int argc,char* argv[])
{
	//register all extensions
	register_all_net_ext();
///////////////////////////////////	
	OOLUA::Script lua ;
	register_api(lua);
	register_all_class(lua);

	try
	{
		if(argc == 2)
		{
				if(!(lua.run_file(argv[1]) && lua.call("main"))) 
				{
						fprintf(stderr,"Failed to run file:%s \n",argv[1]);
						OOLUA::lua_stack_dump(lua);
						return 1;
				}
				printf("run standalone \n");
				return 0;

		}
		if(argc != 3)
		{
			std::cerr << "Usage: pbr_ext <host> <port>"<<std::endl;
			return 1;
		}
		boost::asio::io_service io_service;
		render_client c(lua,io_service,argv[1],argv[2]);
		io_service.run();

	}
	catch(std::exception& e)
	{std::cerr<< e.what() <<std::endl;return 1;}
	return 0;
}


