///////////////////////////////////////////////////////////////////////////////
///  @file cpp_member_func.h
///  @remarks Warning this file was generated, edits to it will not persist if 
///  the file is regenerated.
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence 
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////
#ifndef CPP_MEMBER_FUNC_H_
#	define CPP_MEMBER_FUNC_H_

#	include "param_traits.h"
#ifdef _MSC_VER 
#	define MSC_PUSH_DISABLE_CONDTIONAL_CONSTANT \
	__pragma(warning(push)) \
	__pragma(warning(disable : 4127)) 
#	define MSC_POP_COMPILER_WARNING \
	__pragma(warning(pop)) 
#else
#	define MSC_PUSH_DISABLE_CONDTIONAL_CONSTANT 
#	define MSC_POP_COMPILER_WARNING 
#endif
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

//#include "boost/mpl/accumulate.hpp"
//#include "boost/mpl/accumulate.hpp"
//#include "boost/mpl/accumulate.hpp"
//#include "boost/mpl/accumulate.hpp"

#include "boost/mpl/limits/list.hpp"
#include "boost/fusion/adapted.hpp"
#include "boost/fusion/sequence/intrinsic.hpp"
#include "member_func_helper.h"
//
//#define LUA_CLASS_MEMBER_FUNCTION_2(func_name,return_value,func,P1,P2,mod)\
//	int func_name(lua_State* const l)mod\
//{\
//	assert(m_this);\
//	OOLUA_PARAMS_INTERNAL_2(P1,P2)\
//	typedef param_type<return_value > R;\
//	static R::type (class_::*f )(P1_::type,P2_::type)mod  = &class_::func;\
//	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_>(l,m_this,f,p1,p2);\
//	OOLUA_BACK_INTERNAL_2\
//	return total_out_params< Type_list<out_p<return_value >,P1_,P2_ >::type> ::out;\
//}
#define LUA_EXPORT_FUNC(FT,FN)\
	static int l_##FN(lua_State* l)\
	{\
		typedef boost::function_traits<FT>::result_type result_type;		\
		typedef param_type<result_type> R;\
		typedef func_param_type_list<FT>::type parameter_list;\
		{\
			internal_param_pull2_cpp_push2_lua<parameter_list> scope_value(l);\
			OOLUA::Proxy_caller<R,void>::call<parameter_list>(l,&FN,scope_value.v);\
		}\
		typedef boost::mpl::transform<parameter_list,to_param_type<boost::mpl::_1> >::type params_list;\
		return out_params_count<boost::mpl::push_front<\
		params_list,\
		out_p<result_type>\
		>::type >::value;\
	}\

#define REGISTER_FUNC(L,FN)\
   lua_pushcfunction(L,l_##FN);\
   lua_setglobal(L,#FN);\


#define LUA_CLASS_MEM_FUNC_RENAME(func_type,func_rename,func_name,mod)\
int func_rename(lua_State* const l)mod\
{\
	enum{func_is_ = 0,func_is_const=1};\
	typedef boost::function_traits<func_type>::result_type result_type;\
	typedef param_type<result_type> R;\
	typedef func_param_type_list<func_type>::type parameter_list;\
	{\
	internal_param_pull2_cpp_push2_lua<parameter_list> scope_value(l);\
	static add_member_func_pointer<class_,func_type,1 == BOOST_PP_CAT(func_is_ , mod)>::type f = &class_::func_name;\
	OOLUA::Proxy_caller<R,class_>::call<parameter_list>(l,m_this,f,scope_value.v);\
	}\
	typedef boost::mpl::transform<parameter_list,to_param_type<boost::mpl::_1> >::type params_list;\
	return out_params_count< boost::mpl::push_front<\
	params_list,\
	out_p<result_type > \
	>::type >::value;\
}\

#define LUA_CLASS_MEM_FUNC(func_type,func_name,mod)\
	LUA_CLASS_MEM_FUNC_RENAME(func_type,func_name,func_name,mod)\

#define LUA_MEM_FUNC(func_type,func_name)\
	LUA_CLASS_MEM_FUNC(func_type,func_name,)\

#define LUA_MEM_FUNC_CONST(func_type,func_name)\
	LUA_CLASS_MEM_FUNC(func_type,func_name,const)

#define LUA_MEM_FUNC_RENAME(func_type,func_rename,func_name)\
	LUA_CLASS_MEM_FUNC_RENAME(func_type,func_rename,func_name,)\


#define LUA_MEM_FUNC_RENAME_CONST(func_type,func_rename,func_name)\
	LUA_CLASS_MEM_FUNC_RENAME(func_type,func_rename,func_name,const)\

namespace OOLUA{
	template<typename I,typename T>
	struct out_params_add{
		typedef boost::mpl::int_<I::value + T::out> type;
	};
	template<typename Seq>
	struct out_params_count{
		typedef typename 
			boost::mpl::accumulate<Seq,boost::mpl::int_<0>,
			out_params_add<boost::mpl::_1,boost::mpl::_2> >::type result_type;
		enum{value = result_type::value};
	};
	template<class C,typename FuncT,int arity,bool is_const>
	struct add_member_func_pointer_type_impl;
	template<typename FuncT,int arity>
	struct func_parameter_type_list_impl;

#   define MA_PP_FUNC_ARGS_AUX(z, N, FuncT) \
	typedef typename param_type<typename boost::function_traits<FuncT>::BOOST_PP_CAT(BOOST_PP_CAT(arg,BOOST_PP_INC(N)), _type)>::type  arg_type##N; \

#define  MA_PP_FUNC_PARAM_ARGS_AUX(z,N,FuncT)\
	typedef typename boost::function_traits<FuncT>::BOOST_PP_CAT(BOOST_PP_CAT(arg,BOOST_PP_INC(N)), _type)  arg_type##N; \

#define MA_PP_ADD_MEMBER_FUNC_POINTER(z,N,_)\
	template<typename FuncT>\
	struct func_parameter_type_list_impl<FuncT,N>{\
	BOOST_STATIC_ASSERT((N == boost::function_traits<FuncT>::arity));\
	BOOST_PP_REPEAT( \
	N \
	, MA_PP_FUNC_PARAM_ARGS_AUX \
	, FuncT \
	) \
	typedef boost::mpl::list<BOOST_PP_ENUM_PARAMS(N,arg_type)> type;\
	};\
	template<class C,typename FuncT>\
	struct add_member_func_pointer_type_impl<C,FuncT,N,false>\
	{\
	BOOST_STATIC_ASSERT((N == boost::function_traits<FuncT>::arity));\
	BOOST_PP_REPEAT( \
	N \
	, MA_PP_FUNC_ARGS_AUX \
	, FuncT \
	) \
	typedef typename param_type<typename boost::function_traits<FuncT>::result_type>::type result_type;\
	typedef result_type (C::* type)(BOOST_PP_ENUM_PARAMS(N,arg_type));\
	};\
	template<class C,typename FuncT>\
	struct add_member_func_pointer_type_impl<C,FuncT,N,true>\
	{\
	BOOST_STATIC_ASSERT((N == boost::function_traits<FuncT>::arity));\
	BOOST_PP_REPEAT( \
	N \
	, MA_PP_FUNC_ARGS_AUX \
	, FuncT \
	) \
	typedef typename param_type<typename boost::function_traits<FuncT>::result_type>::type result_type;\
	typedef result_type (C::* type)(BOOST_PP_ENUM_PARAMS(N,arg_type))const;\
	};\

#ifndef MA_FUNCTION_MAX_ARG_NUM
#define MA_FUNCTION_MAX_ARG_NUM 10
#endif

	BOOST_PP_REPEAT(MA_FUNCTION_MAX_ARG_NUM,MA_PP_ADD_MEMBER_FUNC_POINTER,_)

#undef MA_PP_ADD_MEMBER_FUNC_POINTER
#undef MA_PP_FUNC_ARGS_AUX

	template<class C,typename FuncT,bool is_const>
	struct add_member_func_pointer:add_member_func_pointer_type_impl<C,FuncT,boost::function_traits<FuncT>::arity,is_const>{
	};
	template<typename FuncT>
	struct func_param_type_list:func_parameter_type_list_impl<FuncT,boost::function_traits<FuncT>::arity>
	{};
	template<typename T>
	struct to_pull_type{
		typedef param_type<T> parameter_type;
		typedef typename parameter_type::pull_type type;
	};
	template<typename T>
	struct to_param_type{
		typedef param_type<T> type;
	};
	template<typename TypeSeq>
	struct internal_param_pull2_cpp_push2_lua{
		typedef typename boost::mpl::transform<TypeSeq,to_param_type<boost::mpl::_1> >::type param_type_list;
		typedef typename boost::mpl::transform<TypeSeq,to_pull_type<boost::mpl::_1> >::type pull_type_list;
		typedef typename  boost::fusion::result_of::as_vector<pull_type_list>::type vector_type;
		/*static*/ vector_type v;
		lua_State* const l;

		template<int idx>
		struct pull2cpp{
			void operator()(vector_type& v,lua_State* const l)const
			{
				BOOST_STATIC_ASSERT((!boost::mpl::empty<TypeSeq>::value));
				typedef typename boost::mpl::at_c<TypeSeq,boost::mpl::size<TypeSeq>::value - 1 - idx>::type cur_type;
				typedef param_type<cur_type> parameter_type;
				//typedef cur_type parameter_type;
				MSC_PUSH_DISABLE_CONDTIONAL_CONSTANT
				if(parameter_type :: in)
					OOLUA::Member_func_helper<parameter_type,parameter_type::owner>::pull2cpp(l,boost::fusion::at_c<boost::mpl::size<TypeSeq>::value - 1 - idx>(v));
				MSC_POP_COMPILER_WARNING
			}
		};
		template<int idx>
		struct push2lua{
			void operator()(vector_type& v,lua_State* const l)const
			{
				typedef typename boost::mpl::at_c<TypeSeq,idx>::type cur_type;
				typedef param_type<cur_type> parameter_type;
				//typedef cur_type parameter_type;
				MSC_PUSH_DISABLE_CONDTIONAL_CONSTANT
				if (parameter_type::out)
					OOLUA::Member_func_helper<parameter_type,parameter_type::owner>::push2lua(l,boost::fusion::at_c<idx>(v));
				MSC_POP_COMPILER_WARNING
			}
		};
		template<int b,int e,template<int> class F>
		struct for_each{
			BOOST_STATIC_ASSERT(b<e);
			static void apply(vector_type& v,lua_State* const l)
			{
				F<b>()(v,l);
				for_each<b+1,e,F>::apply(v,l);
			}
		};
		template<int e,template<int> class F>
		struct for_each<e,e,F>{ static void apply(vector_type&,lua_State* const){}};

		internal_param_pull2_cpp_push2_lua(lua_State* const ll):l(ll)
		{
			for_each<0,boost::mpl::size<param_type_list>::value,pull2cpp>::apply(v,l);
		}
		~internal_param_pull2_cpp_push2_lua(){
			for_each<0,boost::mpl::size<param_type_list>::value,push2lua>::apply(v,l);
		}
	private:
		internal_param_pull2_cpp_push2_lua& operator=(const internal_param_pull2_cpp_push2_lua&);
	};
}


#endif 
