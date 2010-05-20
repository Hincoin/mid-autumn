#include "net.hpp"
#include "dispatcher.hpp"
#include "serialization.hpp"

#include "DefaultConfigurations.hpp"
#include "rpc_svr.hpp"
#include "pbr_svr.hpp"
#include "render_node.hpp"

namespace ma{
	
		namespace rpc{

struct svr_send_rpc_handler:net::connection_write_handler_base
{
	typedef void result_type;
	svr_send_rpc_handler(render_node& ctx):rpc_ctx(ctx){}
	//save rpc context reference in this handler
	result_type operator()(const boost::system::error_code& e,std::size_t transfer = 0)
	{
		if(e){
			rpc_ctx.disconnect();//handle error: disconnect it or log it
			std::cerr<<e.message()<<std::endl;
		}
	}
	render_node& rpc_ctx;
};


namespace c2s{

	void request_render_task(conn_t c)
		{
			render_node* r = (render_node*)c->get_context();
			//get svr and do somthing
			//unpack the arguments and call real function
			printf("new client request render task\n");
			//send_rpc<s2c::rpc_preprocess>(net::connection_write_handler_ptr(new svr_send_rpc_handler(*r)),c);
			crop_window* win=0;
			if(r->get_svr().getNextCropWindow(*r,win))
				send_rpc<s2c::rpc_render_crop>(net::connection_write_handler_ptr(new svr_send_rpc_handler(*r)),c,*win);
			else
			{
				//end frame
				//remove the crop window of finished
				pbr_svr& svr = r->get_svr();
				svr.end_frame();	
			}
		}
			void add_sample(conn_t conn,camera_sample_t sample,ray_t ray,spectrum_t l,float alpha)
			{
				get_film()->addSample(sample,ray,l,alpha);
			}
			void write_image(conn_t conn)
			{
				get_film()->writeImage();
			}


}
}
	void render_node::start_current_frame()
	{
		status_=START_FRAME;
		rpc::send_rpc<rpc::s2c::rpc_start_frame>(net::connection_write_handler_ptr(new rpc::svr_send_rpc_handler(*this)),connection_,cur_frame_);
	}
}

