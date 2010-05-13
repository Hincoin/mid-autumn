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
			void render_crop(conn_t,float,float,float,float);
		}
			
		struct rpc_func_wrapper const s2c_func_tbl[]
		=
		{
			//the index is for startup check to ensure the index is same to the index in func_tbl
			{s2c::rpc_preprocess::index,&rpc_function<s2c::rpc_preprocess::function_type>,(func_handle_t)&s2c::preprocess}
			,{s2c::rpc_render_crop::index,&rpc_function<s2c::rpc_render_crop::function_type>,(func_handle_t)&s2c::render_crop}
			,{-1,0,0}//terminal
			//,
		};
}


}

#endif
