#ifndef _MA_INCLUDED_RPC_SVR_HPP_
#define _MA_INCLUDED_RPC_SVR_HPP_

#include "pbr_rpc.hpp"
namespace ma
{
namespace rpc
{
		//callee side specific
		namespace c2s{
		
			void request_render_task(conn_t);

		}
			
		struct rpc_func_wrapper const c2s_func_tbl[]
		=
		{
			//the index is for startup check to ensure the index is same to the index in func_tbl
			{c2s::rpc_request_render_task::index,&rpc_function<c2s::rpc_request_render_task::function_type>,(func_handle_t)&c2s::request_render_task}
			,{-1,0,0}//terminal
			//,
		};

}

}

#endif 
