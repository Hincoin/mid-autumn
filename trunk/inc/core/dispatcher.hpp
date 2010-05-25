#ifndef _MA_INCLUDED_RPC_DISPATCHER_HPP_
#define _MA_INCLUDED_RPC_DISPATCHER_HPP_
#include <boost/type_traits.hpp>
#include "boost/preprocessor/cat.hpp"
#include "boost/preprocessor/enum.hpp"
#include "boost/preprocessor/enum_params.hpp"
#include "boost/preprocessor/enum_shifted_params.hpp"
#include "boost/preprocessor/repeat.hpp"
#include "boost/mpl/accumulate.hpp"
#include "boost/mpl/int.hpp"
#include "boost/mpl/transform.hpp"
#include "boost/mpl/vector.hpp"
#include "boost/mpl/at.hpp"
#include "boost/mpl/list.hpp"
#include <boost/fusion/container.hpp>

#include "boost/mpl/limits/list.hpp"
#include "boost/fusion/adapted.hpp"
#include "boost/fusion/include/mpl.hpp"
#include "boost/fusion/container/vector/convert.hpp"
#include "boost/fusion/include/as_vector.hpp"
#include "boost/fusion/sequence/intrinsic.hpp"
#include "boost/ref.hpp"

#include "serialization.hpp"
#include "net.hpp"

#include <cstring>


namespace ma
{
	namespace rpc
	{

struct rpc_msg_t
{
	size_t size;
	void* buff;
};
typedef net::connection_ptr conn_t;
	}
}
namespace ma{
	namespace rpc{
		namespace detail{

	template<typename FuncT,int N>
	struct func_parameter_type_list_impl;

#define  RPC_PP_FUNC_PARAM_ARGS_AUX(z,N,FuncT)\
	typedef typename boost::function_traits<FuncT>::BOOST_PP_CAT(BOOST_PP_CAT(arg,BOOST_PP_INC(N)), _type)  arg_type##N; \

#define RPC_PP_FUNCTION_TRAITS(z,N,_)\
	template<typename FuncT>\
	struct func_parameter_type_list_impl<FuncT,N>{\
	BOOST_STATIC_ASSERT((N == boost::function_traits<FuncT>::arity));\
	BOOST_PP_REPEAT( \
	N \
	, RPC_PP_FUNC_PARAM_ARGS_AUX \
	, FuncT \
	) \
	typedef boost::mpl::vector<BOOST_PP_ENUM_SHIFTED_PARAMS(N,arg_type)> arg_types;\
	typedef boost::fusion::vector<BOOST_PP_ENUM_SHIFTED_PARAMS(N,arg_type)> arg_tuple;		\
	};\


#ifndef MA_FUNCTION_MAX_ARG_NUM
#define MA_FUNCTION_MAX_ARG_NUM 10
#endif

	BOOST_PP_REPEAT(MA_FUNCTION_MAX_ARG_NUM,RPC_PP_FUNCTION_TRAITS,_)

#undef RPC_PP_FUNCTION_TRAITS
#undef RPC_PP_FUNC_PARAM_ARGS_AUX
		}

		template<typename FuncT>
		struct rpc_func_param_type_list:detail::func_parameter_type_list_impl<FuncT,boost::function_traits<FuncT>::arity>
	{};

/*BOOST_MPL_ASSERT((boost::is_same<
			boost::fusion::vector<int,char>,
			boost::fusion::result_of::as_vector<boost::mpl::vector<in,char> > ::type
			>));
			*/

#define RPC_PARAM_CONVERTER_AUX(z,N,_)\
		typedef typename boost::mpl::at_c<Seq,N>::type param_##N##_type;\
		typedef typename boost::remove_reference<param_##N##_type>::type no_ref_param_##N##_type;\
		typedef typename boost::remove_const<no_ref_param_##N##_type>::type raw_type##N;\
		typedef typename boost::add_reference<raw_type##N>::type ref_type##N;\
	 	ref_type##N p_##N = const_cast<ref_type##N>(boost::fusion::at_c<N>(p));\
		serialization::deserialize((p_##N),in);

#define RPC_CALL_FUNC_AUX(z,N,_)\
	template<typename Seq, BOOST_PP_ENUM_PARAMS(N,typename T) BOOST_PP_COMMA_IF(N) typename FuncType>\
	void call_rpc_function(std::istream& in,conn_t conn,FuncType ptr2fun,\
	 boost::fusion::vector<BOOST_PP_ENUM_PARAMS(N,T)>& p)\
		{\
			BOOST_PP_REPEAT(N,RPC_PARAM_CONVERTER_AUX,_);\
			return (*ptr2fun)(conn BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N,p_));\
		}\

#ifndef MA_FUNCTION_MAX_ARG_NUM
#define MA_FUNCTION_MAX_ARG_NUM 10
#endif
BOOST_PP_REPEAT(MA_FUNCTION_MAX_ARG_NUM,RPC_CALL_FUNC_AUX,_)
#undef RPC_CALL_FUNC_AUX
#undef RPC_PARAM_CONVERTER_AUX

typedef void(*func_handle_t)();

template<typename Function>
void rpc_function(func_handle_t f,conn_t conn,const rpc_msg_t& msg)
{
	Function* func = reinterpret_cast<Function*>(f);
	typedef typename rpc_func_param_type_list<Function>::arg_types parameter_list;
	typedef typename rpc_func_param_type_list<Function>::arg_tuple arg_tuple;
	serialization::rpc_streambuf buff((char*)msg.buff,msg.size);
	std::istream in(&buff);		
	arg_tuple params;

	try{
		call_rpc_function<parameter_list>(in,conn,func,params);
	}
	catch(serialization::serialization_error&)
	{
		//do nothing by default
		std::cerr<<"rpc function serialization failed!"<<std::endl;
	}
}
		struct rpc_func_wrapper 
		{
			typedef void (*rpc_func_t)(func_handle_t,conn_t,const rpc_msg_t&);
			int index;
			rpc_func_t f;
			func_handle_t func_ptr;//actual function handle
			
		/*	
			template<typename Function>
			rpc_func_wrapper(int fid,Function ff):id(fid),f(&rpc_function<Function>),func_ptr(ff)			
			{}
			*/
			void operator()(conn_t conn,rpc_msg_t msg)const
			{
				(*f)(func_ptr,conn,msg);
			}
		};	

template<typename FunInfo >
void send_rpc(net::connection_write_handler_ptr handler,conn_t conn)
{
	using namespace serialization;
	std::ostringstream out;
	try
	{
		serialize((int)FunInfo::index,out);
	}
	catch (serialization_error& e)
	{
		boost::system::error_code e(boost::asio::error::invalid_argument);
		(*handler)(e);
	}
	const std::string& o_str = out.str();
	conn->async_write(force_move(o_str),handler);
}
#define DEF_SEND_RPC_AUX(z,N,_)\
template<typename FunInfo BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N,typename T) >\
void send_rpc(net::connection_write_handler_ptr handler,conn_t conn BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_BINARY_PARAMS(N,const T, &arg))\
{\
	using namespace serialization;\
	std::ostringstream out;\
	try\
	{\
		typedef typename FunInfo::arg_tuple arg_tuple;\
		arg_tuple v(BOOST_PP_ENUM_PARAMS(N,arg));\
		serialize((int)FunInfo::index,out);\
		serialize(v,out);\
	}\
	catch (serialization_error& e)\
	{\
		boost::system::error_code e(boost::asio::error::invalid_argument);\
		(*handler)(e);\
	}\
	const std::string& o_str = out.str();\
	conn->async_write(force_move(o_str),handler);\
}

BOOST_PP_REPEAT_FROM_TO(1,MA_FUNCTION_MAX_ARG_NUM,DEF_SEND_RPC_AUX,_)

#undef DEF_SEND_RPC_AUX

template<int N>
bool receive_rpc(conn_t conn,const rpc_msg_t& m,const rpc_func_wrapper (&func_tbl)[N])
{
	using namespace serialization;
	rpc_streambuf buf((char*)m.buff,m.size);
	std::istream in(&buf);
	try {
		int func_id = -1;
		deserialize(func_id,in);
		
		if(func_id != -1 && (func_id) < N)
		{
			rpc_msg_t func_arg;
			func_arg.buff = (char*)m.buff + sizeof(func_id);
			func_arg.size = int(m.size - sizeof(func_id));
			func_tbl[func_id](conn,func_arg);	
		}
		else
		{
			std::cerr<< "invalid rpc function id\n" <<std::endl;
		}
	}
	catch(serialization_error& )
	{
		return false;	
	}
	return true;
}
//a default do nothing handler for client
struct rpc_null_handler:public net::connection_write_handler_base	
{
	typedef void result_type;
	result_type operator()(const boost::system::error_code& e,std::size_t transfer )
	{
		if(e)
		{
			std::cerr<< e.message() <<std::endl;
		}
	}
};
	}
	
}


#endif
