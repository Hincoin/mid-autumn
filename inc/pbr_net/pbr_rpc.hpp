#ifndef _MA_INCLUDED_PBR_RPC_HPP_
#define _MA_INCLUDED_PBR_RPC_HPP_ 

#include "net.hpp"
#include "dispatcher.hpp"
namespace ma
{
	namespace rpc
	{
		//rpc function declare
		template<typename F,int Idx>
			struct rpc_function_info_t
			{
				enum{index = Idx};
				typedef F function_type;
				typedef typename rpc_func_param_type_list<F>::arg_tuple arg_tuple;
			};
	namespace c2s{
		//caller and callee common typedefs
		typedef rpc_function_info_t<void(conn_t),0> rpc_request_render_task;
		//...
	}	
	namespace s2c{
		typedef rpc_function_info_t<void (conn_t),0> rpc_preprocess;//do preprocess	
		typedef rpc_function_info_t<void (conn_t,float,float,float,float),1> rpc_render_crop;
	}
	}
}


#endif
