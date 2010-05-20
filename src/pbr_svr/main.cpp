#include <deque>
#include <set>
#include <boost/lexical_cast.hpp>
#include "DefaultConfigurations.hpp"
#include "rpc_svr.hpp"
#include "render_node.hpp"
#include "pbr_svr.hpp"
#include "script.hpp"

using namespace ma;
using namespace rpc;
using namespace net;

int main(int argc,char* argv[])
{
	OOLUA::Script lua ;
	register_api(lua);
	register_all_class(lua);
	
	//rpc_c2s
	//func_tbl[0](0,rpc_msg_t());
	//send_rpc<rpc_test_func_info_t>(send_rpc_handler(),rpc::conn_t(),10,'c');
	try {
		if(argc != 3)
		{
			std::cerr << "Usage: pbr_svr <script> <port>" <<std::endl;
			return 1;
		}
	
		lua.run_file(argv[1]);
		if(!(lua && lua.call("startup",false)))
		{
			OOLUA::lua_stack_dump(lua);
			return 1;
		}

		get_renderer();
		unsigned short port = boost::lexical_cast<unsigned short>(argv[2]);
		boost::asio::io_service io_service;
		pbr_svr svr(lua,io_service,port);
		io_service.run();

		if(!(lua && lua.call("cleanup")))
		{
			OOLUA::lua_stack_dump(lua);
			return 1;
		}
	}
	catch(std::exception& e)
	{
		std::cerr<<e.what()<<std::endl;
	}
	return 0;
}
