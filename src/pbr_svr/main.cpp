#include <deque>
#include <set>
#include <boost/lexical_cast.hpp>
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
		if(argc != 2)
		{
			std::cerr << "Usage: pbr_svr <port>" <<std::endl;
			return 1;
		}
	
		unsigned short port = boost::lexical_cast<unsigned short>(argv[1]);
		boost::asio::io_service io_service;
		pbr_svr svr(lua,io_service,port);
		io_service.run();
	}
	catch(std::exception& e)
	{
		std::cerr<<e.what()<<std::endl;
	}
	return 0;
}
