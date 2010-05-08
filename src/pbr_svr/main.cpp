#include <deque>
#include <set>
#include <boost/lexical_cast.hpp>
#include "net.hpp"
#include "dispatcher.hpp"
#include "serialization.hpp"
#include "rpc_svr.hpp"

using namespace ma;
using namespace rpc;
using namespace net;

struct rpc_context
{
	std::deque<conn_t> send_queue;
	std::set<conn_t> connections;
};
struct send_rpc_handler
{
	typedef void result_type;
	send_rpc_handler(rpc_context& ctx):rpc_ctx(ctx){}
	//save rpc context reference in this handler
	result_type operator()(const boost::system::error_code& e,std::size_t transfer = 0)
	{
		assert(rpc_ctx.send_queue.size());
		if(e)rpc_ctx.connections.erase(rpc_ctx.send_queue.front());//handle error: disconnect it or log it
	   	rpc_ctx.send_queue.pop_front();	
	}
	rpc_context& rpc_ctx;
};

//represet a client
class render_node;
typedef boost::shared_ptr<render_node> render_node_ptr;
class pbr_svr
{
	private:
		boost::asio::ip::tcp::acceptor acceptor_;
	public:
	pbr_svr(boost::asio::io_service& io_service,unsigned short port);
	///handle accept
	void handle_accept(const boost::system::error_code& e,
			conn_t conn);
	void add_render_node(render_node_ptr r){ connections_.insert(r);}
	void disconnect_render_node(render_node_ptr r){connections_.erase(r);}
	//svr logic here
	
	private:
	std::set<render_node_ptr> connections_;
};
class render_node:public boost::enable_shared_from_this<render_node>
{
	conn_t connection_;
	pbr_svr& svr_;
	rpc_msg_t msg_;
	//this client status
	public:
	explicit render_node(conn_t conn,pbr_svr& svr ):connection_(conn),svr_(svr){
		connection_->set_context(this);
	}
	void start()
	{
		svr_.add_render_node(shared_from_this());
		connection_->async_read(msg_.size,msg_.buff,boost::bind(&render_node::handle_read,this,
							boost::asio::placeholders::error
						));		
	}

	void handle_read(const boost::system::error_code& e)
	{
		if(!e)
		{
			//ok to call rpc
			printf("msg size %d\n",msg_.size);
			//deserialize and call rpc 
			if(!rpc::receive_rpc(connection_,msg_,rpc::c2s_func_tbl))
			{
				std::cerr<< "receive_rpc failed\n" <<std::endl;
				svr_.disconnect_render_node(shared_from_this());
			}
			else
			{
				//continue to read data from client
				connection_->async_read(msg_.size,msg_.buff,boost::bind(&render_node::handle_read,this,
							boost::asio::placeholders::error
						));		
			}
		}
		else
		{
			svr_.disconnect_render_node(shared_from_this());
		}
	}

};
pbr_svr::pbr_svr(boost::asio::io_service& io_service,unsigned short port):acceptor_(io_service,
			boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port))
	{
		//start an accept operation for a new connection
		conn_t new_conn (new Connection(acceptor_.io_service()));
		acceptor_.async_accept(new_conn->socket(),
				boost::bind(&pbr_svr::handle_accept,this,
					boost::asio::placeholders::error,new_conn));
	}
	///handle accept
	void pbr_svr::handle_accept(const boost::system::error_code& e,
			conn_t conn)
	{
		if(!e)
		{
			//add to all connection set, and handle read data from this connection
			render_node_ptr r( new render_node(conn,*this));
			r->start();
			//start an accept operation for new connection
			conn_t new_conn(new Connection(acceptor_.io_service()));
			acceptor_.async_accept(new_conn->socket(),
					boost::bind(&pbr_svr::handle_accept,this,
						boost::asio::placeholders::error,new_conn)
					);
		}
	}
namespace ma{namespace rpc{
namespace c2s{

	void request_render_task(conn_t c)
		{
			//unpack the arguments and call real function
			printf("new client request render task\n");
			send_rpc<s2c::rpc_preprocess_func_info>(net::connection_write_handler_ptr(new rpc_null_handler()),c);
		}
}
}}
int main(int argc,char* argv[])
{
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
		pbr_svr svr(io_service,port);
		io_service.run();
	}
	catch(std::exception& e)
	{
		std::cerr<<e.what()<<std::endl;
	}
	return 0;
}
