#ifndef CPP_CONSTRUTOR_H
#define CPP_CONSTRUTOR_H

#include <boost/mpl/copy_if.hpp>
#include <boost/mpl/equal.hpp>
#include <boost/mpl/and.hpp>
#include "cpp_member_func.h"


namespace OOLUA{


	template<typename P,int>
		struct can_userdata_convert_to_type{
			static int valid(lua_State*,int){ return 0;}
		};
	template<typename P>
		struct can_userdata_convert_to_type<P,0>
		{
			static int valid(lua_State* l,int index)
			{
				return OOLUA::INTERNAL::class_from_index<typename P::raw_type>(l,index) != 0;
			}
		};
	template<typename Param>
		int param_is_of_type_one(lua_State* l, int const index)
		{
			int lua_stack_t = lua_type(l,index);
			switch(lua_stack_t)
			{
			
				case LUA_TBOOLEAN:
					return boost::is_same<bool,Param::raw_type>::value;break;
				case LUA_TNUMBER:
					return boost::is_arithmetic<Param::raw_type>::value; break;
				case LUA_TSTRING:
					return boost::is_same<char* , Param::raw_type>::value ||
								 boost::is_same<std::string,Param::raw_type>::value; break;
				case LUA_TUSERDATA:
					return can_userdata_convert_to_type<Param,Param::is_integral>::valid(l,index);
					break;
				default:
								   return 0;
								   break;
			}
		}
	template<int I,typename Seq>
	struct param_is_type_accum{
		typedef typename boost::mpl::at_c<Seq,I>::type cur_type;
		int operator()(lua_State* l)const
		{
			return invoke(l,boost::mpl::bool_<(I+1 < boost::mpl::size<Seq>::type::value ) >());
		}
		int invoke(lua_State* l,boost::mpl::bool_<true>)const
		{
			return param_is_of_type_one<cur_type>(l,I) && param_is_type_accum<I+1,Seq>()(l);
		}
		int invoke(lua_State* l,boost::mpl::bool_<false>)const
		{
			return param_is_of_type_one<cur_type>(l,I); 
		}
	};
	template<typename Params>
		int param_is_of_type(lua_State* l)
		{
			return param_is_type_accum<0,Params>()(l);
		}

template<typename Class,typename Params>
	struct Construtor{
		typedef typename boost::mpl::transform<Params,to_param_type<boost::mpl::_1> >::type params_with_traits;
		static int construct(lua_State* l)
		{
			return construct_(l,boost::mpl::bool_<boost::mpl::size<Params>::value == 0>());
		}
		static int construct_(lua_State* l,boost::mpl::bool_<true>)
		{
			construct_impl(l);
			return 1;
		}
		static int construct_(lua_State* l,boost::mpl::bool_<false>)
		{
			if ( param_is_of_type<params_with_traits>(l) )
			{
				construct_impl(l);
				return 1;
			}	
			return 0;
		
		}
		static void construct_impl(lua_State* l)
		{
			{
				internal_param_pull2_cpp_push2_lua<Params> pull_push(l);
				Class* obj = call_construct<Params>(l,pull_push.v);			
				OOLUA::INTERNAL::Lua_ud* ud = OOLUA::INTERNAL::add_ptr(l,obj,false);
				ud->gc = true;
			}
		}
#define MA_LUA_CONVERTER_AUX(z,N,_)\
	typedef param_type<typename boost::mpl::at_c<Seq,N>::type> P##N##_T;\
	Converter<typename P##N##_T::pull_type, typename P##N##_T::type> p_##N(boost::fusion::at_c<N>(p));\

#define MA_LUA_PROXY_MEMBER_CALL_AUX(z,N,T)\
	template<typename Seq  BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N,typename P)>\
	static Class* call_construct(lua_State* const l, \
	boost::fusion::vector##N<BOOST_PP_ENUM_PARAMS(N,P)>& BOOST_PP_IF(N,p,)\
	)\
		{\
		BOOST_PP_REPEAT(N,MA_LUA_CONVERTER_AUX,_)\
		return new Class(BOOST_PP_ENUM_PARAMS(N,p_));\
		}\

#ifndef MA_FUNCTION_MAX_ARG_NUM
#define MA_FUNCTION_MAX_ARG_NUM 10
#endif
		BOOST_PP_REPEAT(MA_FUNCTION_MAX_ARG_NUM,MA_LUA_PROXY_MEMBER_CALL_AUX,_)
#undef MA_LUA_PROXY_MEMBER_CALL_AUX
#undef MA_LUA_CONVERTER_AUX
	
	};

	template<typename Class,typename CParams,size_t I,size_t E>
		struct factory_constructor_impl_f
		{
			typedef typename boost::mpl::at_c<CParams,I>::type cur_params;
			int operator()(lua_State* l)const
			{
				if(Construtor<Class,cur_params>::construct(l))
				{
					return 1;
				}
				else if(I + 1 < E)
				{
					return factory_constructor_impl_f<Class,CParams,I+1,E>()(l);
				}
				else
				{
					luaL_error(l,"%s %d %s %s","Could not match",boost::mpl::size<cur_params>::type::value,
							"parameter constructor for type ",Proxy_class<Class>::class_name);
					return 0;
				}
			}	
			
		};
		template<typename Class,typename CParams,size_t I>
		struct factory_constructor_impl_f<Class,CParams,I,I>
		{
			int operator()(lua_State* l)const
			{
				luaL_error(l,"%s %d %s %s","Could not match",0,
							"parameter constructor for type ",Proxy_class<Class>::class_name);
				return 0;
			}	
			
		};
	template <typename Ps1,typename Sz>
	struct SizeEqualTo:boost::mpl::equal_to<boost::mpl::size<Ps1>,Sz>{};

	template<typename Class,typename CParams>
	inline int factory_constructor_impl(lua_State* l)
	{
		lua_remove(l,1);// remove class type
		int param_count = lua_gettop(l);
		switch (param_count){
#define PARAM_CNT_CASE(z,N,_)\
			case N:\
			typedef boost::mpl::if_c< boost::mpl::bool_<(N == 0)>::value && boost::mpl::equal_to<boost::mpl::size<CParams>,boost::mpl::int_<0> >::value,boost::mpl::vector<boost::mpl::vector<> >,boost::mpl::copy_if<CParams,\
				SizeEqualTo<boost::mpl::_1,boost::mpl::int_<N> >,\
				boost::mpl::back_inserter<boost::mpl::vector<> > >::type >::type _##N##_params;\
					return factory_constructor_impl_f<Class,_##N##_params,0,boost::mpl::size<_##N##_params>::type::value>()(l);\
				break;\

#ifndef MA_FUNCTION_MAX_ARG_NUM
#define MA_FUNCTION_MAX_ARG_NUM 10
#endif
		BOOST_PP_REPEAT(MA_FUNCTION_MAX_ARG_NUM,PARAM_CNT_CASE,_)
	
#undef PARAM_CNT_CASE
			default:
				luaL_error(l,"%s %d %s %s","Could not match",param_count,
							"parameter constructor for type ",Proxy_class<Class>::class_name);
				return 0;
				break;
		}
	}
//DEF_CTOR(mpl::vector<>,mpl::vector<int>,mpl::vector<bool>)'
#define LUA_CTOR_ARGS(...)\
	mpl::vector<__VA_ARGS__>

#define LUA_CTORS(...)\
static int factory_constructor(lua_State* l)\
{\
	typedef mpl::vector<__VA_ARGS__> ctor_params;\
	return factory_constructor_impl<class_,ctor_params>(l);\
}

}



/*
 LUA_TBOOLEAN
 LUA_TNUMBER
 LUA_TSTRING
 LUA_TUSERDATA

 LUA_TNIL
 LUA_TLIGHTUSERDATA
 LUA_TTABLE
 LUA_TFUNCTION
 LUA_TTHREAD
 *
 *
 *
 *
 *
 *
 * */

#endif

