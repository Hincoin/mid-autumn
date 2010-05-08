#include <iostream>
#include "script.hpp"
#include "oolua_error.h"

#include "ptr_var.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>
#include "net.hpp"
#include "dispatcher.hpp"
#include "rpc_client.hpp"

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
	if(argc == 1 && ! (lua.run_file("main.lua") && lua.call("main") ))
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
class render_client
{
	public:
	render_client(boost::asio::io_service& io_service,
			const std::string& host,
			const std::string& service)	
		:connection_(new Connection(io_service))
	{
		//resolve the host name into an ip address
		using namespace boost::asio::ip;
		tcp::resolver resol(io_service);
		tcp::resolver::query query(host,service);
		tcp::resolver::iterator endpoint_iterator = 
			resol.resolve(query);
		tcp::endpoint endp= *endpoint_iterator;
		

		connection_->socket().async_connect(endp,
				boost::bind(&render_client::handle_connect,
					this,
					boost::asio::placeholders::error,
					++endpoint_iterator));
		
	}
	void handle_connect(const boost::system::error_code& e,
			boost::asio::ip::tcp::resolver::iterator 
			endpoint_iterator)
	{
		if(!e)
		{
			onconnected();		
			connection_->async_read(msg_.size,msg_.buff,
					boost::bind(&render_client::handle_read,
						this,
						boost::asio::placeholders::error));
		}
		else if(endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
		{
			//try the next endpoint
			connection_->socket().close();
			boost::asio::ip::tcp::endpoint endpoint 
				= *endpoint_iterator;
			connection_->socket().async_connect(endpoint,
					boost::bind(&render_client::handle_connect,
						this,
						boost::asio::placeholders::error,
						++endpoint_iterator));
		}
		else
		{
			std::cerr<< e.message() <<std::endl;
		}
	}
	void handle_read(const boost::system::error_code& e)
	{
		if(!e)
		{
			printf("read data from svr\n");
			//deserialize and call rpc 
			if(!rpc::receive_rpc(connection_,msg_,rpc::s2c_func_tbl))
			{
			}
			else//rpc success to read more
				connection_->async_read(msg_.size,msg_.buff,
					boost::bind(&render_client::handle_read,
						this,
						boost::asio::placeholders::error));
		}
		else
		{
			std::cerr<< e.message()<<std::endl;
		}
	}
	void onconnected()
	{
		connection_->set_context(this);
		rpc::send_rpc<rpc::c2s::rpc_request_render_task_func_info>(net::connection_write_handler_ptr(new rpc::rpc_null_handler()),connection_);
	}
	private:
	conn_t connection_;
	rpc_msg_t msg_;
};

namespace ma{namespace rpc{
namespace s2c{

	void preprocess(conn_t c)
		{
			//unpack the arguments and call real function
			printf("svr request do preprocess\n");
			rpc::send_rpc<rpc::c2s::rpc_request_render_task_func_info>(net::connection_write_handler_ptr(new rpc_null_handler()),c);
		}
}
}}
int main(int argc,char* argv[])
{
	try
	{
		if(argc != 3)
		{
			std::cerr << "Usage: pbr_ext <host> <port>"<<std::endl;
			return 1;
		}

		boost::asio::io_service io_service;
		render_client c(io_service,argv[1],argv[2]);
		io_service.run();
	}
	catch(std::exception& e)
	{}
	return 0;
}


