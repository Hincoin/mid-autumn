#ifndef _MA_INCLUDED_PBR_RPC_HPP_
#define _MA_INCLUDED_PBR_RPC_HPP_ 

#include "net.hpp"
#include "dispatcher.hpp"
#include "DefaultConfigurations.hpp"

namespace ma
{
	struct crop_window:public serialization::serializable<crop_window>{
		int frame;
		int id;
		float xmin;
		float xmax;
		float ymin;
		float ymax;
		crop_window(int f=-1,int i=-1,float x_min=0.f,float x_max=1.f,float y_min=0.f,float y_max=1.f)
			:frame(f),id(i),xmin(x_min),xmax(x_max),ymin(y_min),ymax(y_max)
		{
		}
		void serializeImpl(std::ostream& out)const
		{
			serialization::serialize(frame,out);
			serialization::serialize(id,out);
			serialization::serialize(xmin,out);
			serialization::serialize(xmax,out);
			serialization::serialize(ymin,out);
			serialization::serialize(ymax,out);
		}
		void deserializeImpl(std::istream& in)
		{
			using namespace serialization;
			serialization::deserialize(frame,in);
			serialization::deserialize(id,in);
			serialization::deserialize(xmin,in);
			serialization::deserialize(xmax,in);
			serialization::deserialize(ymin,in);
			serialization::deserialize(ymax,in);
		}
	};
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
		typedef rpc_function_info_t<void(conn_t,camera_sample_t,ray_t,spectrum_t,float),1> rpc_add_sample;
		typedef rpc_function_info_t<void(conn_t),2> rpc_write_image;
	
		//...
	}	
	namespace s2c{
		typedef rpc_function_info_t<void (conn_t),0> rpc_preprocess;//do preprocess	
		typedef rpc_function_info_t<void (conn_t, crop_window),1> rpc_render_crop;
		typedef rpc_function_info_t<void(conn_t,int),2> rpc_start_frame;
		typedef rpc_function_info_t<void(conn_t,int),3> rpc_end_frame;
}
	}
}


#endif
