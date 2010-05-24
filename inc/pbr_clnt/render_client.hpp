#ifndef _MA_INCLUDED_RENDER_CLIENT_HPP_
#define _MA_INCLUDED_RENDER_CLIENT_HPP_ 
#include "net.hpp"
#include "dispatcher.hpp"
#include "rpc_client.hpp"

#include "script.hpp"
#include "oolua_error.h"

#include "DefaultConfigurations.hpp"
namespace ma{
class render_client
{
	public:
	render_client(OOLUA::Script& lua,boost::asio::io_service& io_service,
			const std::string& host,
			const std::string& service)	
		:lua_(lua),connection_(new net::Connection(io_service))
	{
		assert(self == 0);
		self = (this);
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
			//deserialize and call rpc 
			if(!rpc::receive_rpc(connection_,msg_,rpc::s2c_func_tbl))
			{
				printf("call rpc failed\n");
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
		//svr will distribute new task for this client
		//rpc::send_rpc<rpc::c2s::rpc_request_render_task>(net::connection_write_handler_ptr(new rpc::rpc_null_handler()),connection_);
	}
	//client specific interface
	bool render_crop(int frame,int id,float xmin,float xmax,float ymin,float ymax)
	{
		scene_ptr renderer = get_renderer();
		if(renderer){
		   	renderer->renderCropWindow(xmin,xmax,ymin,ymax);
			renderer->postRender();
			return true;
		}
		else return false;
	}
	bool frame(int n)
	{
		if(!(lua_ && lua_.call("frame",n)))
		{
			OOLUA::lua_stack_dump(lua_);
			return false;
		}
		scene_ptr renderer = get_renderer();
		if(renderer)
		{
			renderer->preRender();
			return true;
		}
		return false;
	}
	void start_render_scene(const std::string& file);
	void end_render_scene();
	static render_client& get_client(){assert(self);return *self;}
	rpc::conn_t get_connection(){return connection_;}
	private:
	OOLUA::Script& lua_;
	rpc::conn_t connection_;
	rpc::rpc_msg_t msg_;

	static render_client* self;
};

}

#endif
