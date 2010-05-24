#ifndef _MA_INCLUDED_PBR_CTRL_HPP_
#define _MA_INCLUDED_PBR_CTRL_HPP_

#include "net.hpp"
#include "dispatcher.hpp"

#include "script.hpp"
#include "oolua_error.h"

#include "rpc_ctrl.hpp"
namespace ma{
class pbr_ctrl
{
	public:
	pbr_ctrl(OOLUA::Script& lua,boost::asio::io_service& io_service,
			const std::string& host,
			const std::string& service)	
		:lua_(lua),connection_(new net::Connection(io_service))
	{
		assert(!self_);
		self_= this;
		//resolve the host name into an ip address
		using namespace boost::asio::ip;
		tcp::resolver resol(io_service);
		tcp::resolver::query query(host,service);
		tcp::resolver::iterator endpoint_iterator = 
			resol.resolve(query);
		tcp::endpoint endp= *endpoint_iterator;
		

		connection_->socket().async_connect(endp,
				boost::bind(&pbr_ctrl::handle_connect,
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
					boost::bind(&pbr_ctrl::handle_read,
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
					boost::bind(&pbr_ctrl::handle_connect,
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
			//deserialize and call rpc 
			if(!rpc::receive_rpc(connection_,msg_,rpc::s2ctrl_func_tbl))
			{
				printf("call rpc failed\n");
			}
			connection_->async_read(msg_.size,msg_.buff,
					boost::bind(&pbr_ctrl::handle_read,
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
		printf("onconnected\n");
		if(!(lua_ && lua_.call("main")))
		{
			OOLUA::lua_stack_dump(lua_);
		}
	}
	static pbr_ctrl& get_controller(){assert(self_);return *self_;}
	void render_scene(const char* scene_desc);
	private:
	OOLUA::Script& lua_;
	rpc::conn_t connection_;
	rpc::rpc_msg_t msg_;


	static pbr_ctrl* self_;
};
void register_functions(lua_State* );
}

#endif
