#ifndef _MA_INCLUDED_RENDER_NODE_HPP_
#define _MA_INCLUDED_RENDER_NODE_HPP_
#include "pbr_svr.hpp"
#include "dispatcher.hpp"
#include "rpc_svr.hpp"
namespace ma
{
enum RenderNodeStatus{IDLE,START_FRAME,RENDER_CROP,END_FRAME};
enum ClientType{CONTROLLER,RENDERER};

class render_node:public boost::enable_shared_from_this<render_node>
{
	rpc::conn_t connection_;
	pbr_svr& svr_;
	rpc::rpc_msg_t msg_;
	//this client status
	int cur_frame_;
	RenderNodeStatus status_;
	ClientType type_;
	std::string render_scene_;
	public:
	explicit render_node(rpc::conn_t conn,pbr_svr& svr ,int cur_frame):connection_(conn),svr_(svr),cur_frame_(cur_frame),status_(IDLE){
		set_type (RENDERER);
		connection_->set_context(this);
	}
	void start()
	{
		svr_.add_render_node(shared_from_this());
		status_ = IDLE;
		connection_->async_read(msg_.size,msg_.buff,boost::bind(&render_node::handle_read,this,
							boost::asio::placeholders::error
						));		
	}

	void handle_read(const boost::system::error_code& e)
	{
		if(!e)
		{
			//ok to call rpc
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
			std::cerr<< e.message() <<std::endl;
			svr_.disconnect_render_node(shared_from_this());
		}
	}
	void disconnect()
	{
		svr_.disconnect_render_node(shared_from_this());	
	}
	//not net logic here
	pbr_svr& get_svr(){return svr_;}
	void set_frame(int f){cur_frame_ = f;}
	int get_frame()const{return cur_frame_;}
	rpc::conn_t get_conn()const{return connection_;}
	void start_current_frame();
	void end_current_frame();
	void idle(){status_ = IDLE;}
	RenderNodeStatus get_status()const{return status_;}
	void set_status(RenderNodeStatus s){status_ = s;}
	ClientType get_type()const{return type_;}
	void set_type(ClientType type){type_ = type;}
	void set_render_scene(const std::string& s){render_scene_ = s;}
	const std::string& get_render_scene()const{return render_scene_;}
	void render_scene(const std::string& f);
	void end_scene();
};



}


#endif
