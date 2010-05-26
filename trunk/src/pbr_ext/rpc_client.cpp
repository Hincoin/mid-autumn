/*#include "rpc_client.hpp"

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
	void render_crop(conn_t c,float xmin,float xmax,float ymin,float ymax)
	{
			printf("render crop:[%.6f,%.6f,%.6f,%.6f]\n",xmin,xmax,ymin,ymax);
			render_client* render_node = (render_client*)c->get_context();
			if(render_node && render_node->render_crop(xmin,xmax,ymin,ymax))
			{
				//todo: return results to svr	
			}
			else
			{
				//todo:report error to svr
			}
	}
}
}}
*/
