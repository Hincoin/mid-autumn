
#include "rpc_client.hpp"

#include "dispatcher.hpp"
#include "render_client.hpp"

namespace ma{namespace rpc{
namespace s2c{

	void preprocess(conn_t c)
		{
			//unpack the arguments and call real function
			printf("svr request do preprocess\n");
			rpc::send_rpc<rpc::c2s::rpc_request_render_task>(net::connection_write_handler_ptr(new rpc_null_handler()),c);
		}
	void render_crop(conn_t c,crop_window crop_win)
	{
			printf("render crop:[%.6f,%.6f,%.6f,%.6f]\n",crop_win.xmin,crop_win.xmax,crop_win.ymin,crop_win.ymax);

			net_film_ptr net_film = dynamic_cast<net_film_ptr>(get_film());
			if(net_film)net_film->setConnection(c);
			render_client* render_node = (render_client*)c->get_context();
			if(render_node && render_node->render_crop(crop_win.frame,crop_win.id,crop_win.xmin,crop_win.xmax,crop_win.ymin,crop_win.ymax))
			{
				//todo: return results to svr	
				rpc::send_rpc<rpc::c2s::rpc_request_render_task>(net::connection_write_handler_ptr(new rpc::rpc_null_handler()),c);
				///send back all the camera samples we get to svr
				//or save image to get another render crop
			}
			else
			{
				//todo:report error to svr
			}
	}
	void start_frame(conn_t conn,int n)
	{
		render_client* client = (render_client*)conn->get_context();
		printf("start a new frame : %d\n",n);
		if (client && client->frame(n))
		{
			//success and send back request todo render task
			rpc::send_rpc<rpc::c2s::rpc_request_render_task>(net::connection_write_handler_ptr(new rpc::rpc_null_handler()),conn);
		}
		else
		{
			//todo report error or disconnect
		}
	}
	void end_frame(conn_t conn,int n)//destroy world
	{
	}
}
}}

