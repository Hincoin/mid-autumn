#ifndef _MA_INCLUDED_RPC_CTRL_HPP_
#define _MA_INCLUDED_RPC_CTRL_HPP_

#include "pbr_rpc.hpp"



namespace ma
{
	namespace rpc{
namespace s2ctrl{
	void write_image(std::string file,std::string data);
}

		struct rpc_func_wrapper const s2ctrl_func_tbl[]
		=
		{
			//the index is for startup check to ensure the index is same to the index in func_tbl
			{s2ctrl::rpc_write_image::index,&rpc_function<s2ctrl::rpc_write_image::function_type>,(func_handle_t)&s2ctrl::write_image}
			,{-1,0,0}
		};
}
}

#endif
