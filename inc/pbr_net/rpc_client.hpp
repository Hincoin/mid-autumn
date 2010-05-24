#ifndef _MA_INCLUDED_RPC_CLIENT_HPP_
#define _MA_INCLUDED_RPC_CLIENT_HPP_

#include "pbr_rpc.hpp"
namespace ma
{

namespace rpc
{
		//callee side specific
		namespace s2c{
		
			void preprocess(conn_t);
			void render_crop(conn_t,crop_window);
			void start_frame(conn_t,int);
			void end_frame(conn_t,int);
			void start_render_scene(conn_t,std::string);
			void end_render_scene(conn_t);
		}
			
		struct rpc_func_wrapper const s2c_func_tbl[]
		=
		{
			//the index is for startup check to ensure the index is same to the index in func_tbl
			{s2c::rpc_preprocess::index,&rpc_function<s2c::rpc_preprocess::function_type>,(func_handle_t)&s2c::preprocess}
			,{s2c::rpc_render_crop::index,&rpc_function<s2c::rpc_render_crop::function_type>,(func_handle_t)&s2c::render_crop}
			,{s2c::rpc_start_frame::index,&rpc_function<s2c::rpc_start_frame::function_type>,(func_handle_t)&s2c::start_frame}
			,{s2c::rpc_end_frame::index,&rpc_function<s2c::rpc_end_frame::function_type>,(func_handle_t)&s2c::end_frame}
			,{s2c::rpc_start_render_scene::index,&rpc_function<s2c::rpc_start_render_scene::function_type>,(func_handle_t)&s2c::start_render_scene}
			,{s2c::rpc_end_render_scene::index,&rpc_function<s2c::rpc_end_render_scene::function_type>,(func_handle_t)&s2c::end_render_scene}
			,{-1,0,0}//terminal
			//,
		};
}


}

#endif
