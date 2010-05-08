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

		}
			
		struct rpc_func_wrapper const s2c_func_tbl[]
		=
		{
			//the index is for startup check to ensure the index is same to the index in func_tbl
			{s2c::rpc_preprocess_func_info::index,&rpc_function<s2c::rpc_preprocess_func_info::function_type>,(func_handle_t)&s2c::preprocess}
			,{-1,0,0}//terminal
			//,
		};
}


}

#endif
