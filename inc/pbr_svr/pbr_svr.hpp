#ifndef _MA_INCLUDED_PBR_SVR_HPP_
#define _MA_INCLUDED_PBR_SVR_HPP_

#include "net.hpp"
#include "dispatcher.hpp"
#include <set>
#include "rpc_svr.hpp"

#include "oolua.h"
namespace ma{


	class render_node;
	typedef boost::shared_ptr<render_node> render_node_ptr;

class pbr_svr
{
	private:
		boost::asio::ip::tcp::acceptor acceptor_;
	public:
	pbr_svr(OOLUA::Script& lua,boost::asio::io_service& io_service,unsigned short port);
	///handle accept
	void handle_accept(const boost::system::error_code& e,
			rpc::conn_t conn);
	void add_render_node(render_node_ptr r){ connections_.insert(r);}
	void disconnect_render_node(render_node_ptr r){connections_.erase(r);}
	//svr logic here
	bool getNextCropWindow(render_node& r,crop_window* &w);	
	void end_frame();//end current frame
	private:
	typedef std::set<render_node_ptr> connection_set_t;
	std::vector<crop_window> crop_windows_;//
	connection_set_t connections_;
	int cur_frame_;
	int next_frame_;
	OOLUA::Script& lua_;
};


}


#endif
