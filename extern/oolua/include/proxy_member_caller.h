#ifndef PROXY_MEMBER_CALLER_H_
#	define PROXY_MEMBER_CALLER_H_
#include <boost/fusion/container.hpp>

#	include "member_func_helper.h"
#	include "param_traits.h"

namespace OOLUA
{

	template <typename Return, typename Class, int is_void> struct Proxy_caller_impl;
	template <typename R, typename C>
	struct Proxy_caller_impl<R,C, 0 >
	{
		template<typename FuncType>
		static void call(lua_State*  const l,C* m_this, FuncType ptr2mem )
		{
			typename R::type r( (m_this->*ptr2mem)() );
			OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
		}

		//template<typename P0,typename FuncType>
		//static void call(lua_State* const  l,C* m_this,FuncType ptr2mem,typename boost::fusion::tuple<P0>&  p)
		//{
		//	Converter<typename param_type<P0>::pull_type, typename param_type<P0>::type> p0_(boost::fusion::at_c<0>(p));
		//	typename R::type r( (m_this->*ptr2mem) (p0_) );
		//	OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);
		//}
#define MA_LUA_CONVERTER_AUX(z,N,_)\
	typedef param_type<typename boost::mpl::at_c<Seq,N>::type> P##N##_T;\
	Converter<typename P##N##_T::pull_type, typename P##N##_T::type> p_##N(boost::fusion::at_c<N>(p));\

#define MA_LUA_PROXY_MEMBER_CALL_AUX(z,N,T)\
	template<typename Seq, BOOST_PP_ENUM_PARAMS(N,typename P) BOOST_PP_COMMA_IF(N) typename FuncType>\
	static void call(lua_State* const l, C* m_this,FuncType ptr2mem ,\
	 boost::fusion::vector##N<BOOST_PP_ENUM_PARAMS(N,P)>& BOOST_PP_IF(N,p,)\
		)\
		{\
		BOOST_PP_REPEAT(N,MA_LUA_CONVERTER_AUX,_)\
		typename R::type r((m_this->*ptr2mem)(\
		BOOST_PP_ENUM_PARAMS(N,p_)\
		));\
		OOLUA::Member_func_helper<R,R::owner>::push2lua(l,r);\
		}\

#ifndef MA_FUNCTION_MAX_ARG_NUM
#define MA_FUNCTION_MAX_ARG_NUM 10
#endif

		BOOST_PP_REPEAT(MA_FUNCTION_MAX_ARG_NUM,MA_LUA_PROXY_MEMBER_CALL_AUX,_)


#undef MA_LUA_PROXY_MEMBER_CALL_AUX
#undef MA_LUA_CONVERTER_AUX

	};

	template <typename R, typename C>
	struct Proxy_caller_impl<R,C,1>
	{
		template<typename FuncType>
		static void call(lua_State*  const /*l*/,C* m_this, FuncType ptr2mem )
		{
			(m_this->*ptr2mem)();
		}
#define MA_LUA_CONVERTER_AUX(z,N,_)\
	typedef param_type<typename boost::mpl::at_c<Seq,N>::type> P##N##_T;\
	Converter<typename P##N##_T::pull_type, typename P##N##_T::type> p_##N(boost::fusion::at_c<N>(p));\

#define MA_LUA_PROXY_MEMBER_CALL_AUX(z,N,T)\
	template<typename Seq, BOOST_PP_ENUM_PARAMS(N,typename P) BOOST_PP_COMMA_IF(N) typename FuncType>\
	static void call(lua_State* const l, C* m_this,FuncType ptr2mem ,\
	boost::fusion::vector##N<BOOST_PP_ENUM_PARAMS(N,P)>& BOOST_PP_IF(N,p,)\
	)\
		{\
		BOOST_PP_REPEAT(N,MA_LUA_CONVERTER_AUX,_)\
		(m_this->*ptr2mem)(BOOST_PP_ENUM_PARAMS(N,p_));\
		}\

#ifndef MA_FUNCTION_MAX_ARG_NUM
#define MA_FUNCTION_MAX_ARG_NUM 10
#endif
		BOOST_PP_REPEAT(MA_FUNCTION_MAX_ARG_NUM,MA_LUA_PROXY_MEMBER_CALL_AUX,_)
#undef MA_LUA_PROXY_MEMBER_CALL_AUX
#undef MA_LUA_CONVERTER_AUX
	};

	template <typename PReturn, typename Class> struct Proxy_caller
		:Proxy_caller_impl<PReturn,Class,LVD::is_void<typename  PReturn::type >::value>
	{};

}

#endif 
