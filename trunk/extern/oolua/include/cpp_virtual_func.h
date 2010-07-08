#ifndef _OOLUA_CPP_VIRTUAL_FUNCTION_INCLUDE_
#define _OOLUA_CPP_VIRTUAL_FUNCTION_INCLUDE_
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
namespace OOLUA
{
	template<typename T> struct cpp_acquire_base_ptr;
	//the cpp proxy of lua object or passsed from lua side
	template<typename T>
	struct cpp_proxy{
		typedef cpp_acquire_base_ptr<T> type;
	};
	template<typename B> struct make_cpp_proxy;
	
#define MAKE_CPP_PORXY(B,D)\
	namespace OOLUA{\
	template<>\
	struct make_cpp_proxy<B>{\
		typedef cpp_proxy<D>::type type;\
	};\
	}\

	namespace details
	{
			template<typename R>
					struct get_return_value{
							R operator()(lua_State* l,int npop)const
							{
									R r;
									pull2cpp(l,r);
									lua_pop(l,npop);//number of arguments ?
									return r;
							}
					};
			template<>
					struct get_return_value<void>
					{
							void operator()(lua_State* l,int npop)const
							{
									lua_pop(l,npop);//number of arguments ?
							}
					};

	}
	template<typename T>
	void pull2cpp(lua_State* const s, cpp_acquire_base_ptr<T>&  value);
	
	template<typename T>
			struct cpp_acquire_base_ptr{
					private:
				public:
		typedef T type;
		typedef typename Raw_type<T>::type raw;
		typedef typename Pull_type<raw,LVD::is_integral_type<raw>::value>::type pull_type;
		enum { in = 1 };
		enum { out = 0 };
		enum { owner = Cpp };
		enum { is_by_value = Type_enum_defaults<type>::is_by_value  };
		enum { is_constant = Type_enum_defaults<type>::is_constant  };
		enum { is_integral = Type_enum_defaults<type>::is_integral  };

		cpp_acquire_base_ptr(raw* ptr):m_ptr(ptr),called_(false){}
		cpp_acquire_base_ptr():called_(false){m_ptr = (T*)(this);}

		raw* m_ptr;
	protected:
		Lua_ud_ref ud;
		bool called_;

#define _CALL_LUA_FUNCTION_AUX_PUSH2LUA_(z,N,t)\
	push2lua(l,t##N);	\

#define _CALL_LUA_FUNCTION_AUX_(_,NArgs,CallF)\
		template<typename R BOOST_PP_COMMA_IF(NArgs) BOOST_PP_ENUM_PARAMS(NArgs,typename T)>\
		R CallF(const char* func_name BOOST_PP_COMMA_IF(NArgs) BOOST_PP_ENUM_BINARY_PARAMS(NArgs,T,t) , R (T::*f)(BOOST_PP_ENUM_PARAMS(NArgs,T)))\
		{\
			if (called_ || !ud.valid() )\
			{\
				assert(m_ptr);\
				called_=false;\
				return (m_ptr->*f)(BOOST_PP_ENUM_PARAMS(NArgs,t));\
			}\
			assert(ud.valid());\
			called_ = true;\
			lua_State* l = ud.lua_state();\
			INTERNAL::Lua_ud* user_data = INTERNAL::find_ud_dont_care_about_type_and_clean_stack(l,m_ptr);\
			if (!user_data)/*user_data is gc-ed8*/\
			{\
				called_ = false;\
				return (m_ptr->*f)(BOOST_PP_ENUM_PARAMS(NArgs,t));\
			}\
			lua_getref(l,ud.ref());\
\
			OOLUA::push2lua(l,"__index");\
			lua_gettable(l, -2);\
			lua_pushstring(l,func_name);\
			lua_gettable(l,-2);\
			assert(lua_type(l,-1) == LUA_TFUNCTION);\
			lua_pushlightuserdata(l,user_data);\
			BOOST_PP_REPEAT(NArgs,_CALL_LUA_FUNCTION_AUX_PUSH2LUA_,t);\
			if (lua_pcall(l,NArgs+1,1,0))\
			{\
				assert(0);\
			}\
			called_ = false;\
			return details::get_return_value<R>()(l,NArgs - 1);\
			\
		}\

		BOOST_PP_REPEAT(10, _CALL_LUA_FUNCTION_AUX_ , call_lua_func )

		template<typename R,typename T0,typename T1,typename T2>
		R call(const char* func_name,T0 t0,T1 t1,T2 t2,R (T::*f)(T0,T1,T2))//pass default call
		{
			if (called_ || !ud.valid() )
			{
				assert(m_ptr);
				called_=false;
				//default call
				return (m_ptr->*f)(t0,t1,t2);
			}
			assert(ud.valid());
			called_ = true;
			lua_State* l = ud.lua_state();
			int level = lua_gettop(l);
			INTERNAL::Lua_ud* user_data = INTERNAL::find_ud_dont_care_about_type_and_clean_stack(l,m_ptr);
			if (!user_data)//user_data is gc-ed
			{
				called_ = false;
				return (m_ptr->*f)(t0,t1,t2);
			}
			lua_getref(l,ud.ref());

			//printf("%d,",lua_type(l,-1) );
			OOLUA::push2lua(l,"__index");//table key
			lua_gettable(l, -2);//table value
			//printf("%d,",lua_type(l,-1) );
			lua_pushstring(l,func_name);
			lua_gettable(l,-2);
			//is_lua_bind_function
			//if (is_lua_bind_function(l,-1))
			//{
			//	printf("is_lua_bind_function\n");
			//}
			
			bool is_func = lua_type(l,-1) == LUA_TFUNCTION;
			assert(is_func);
			lua_pushlightuserdata(l,user_data);
			push2lua(l,t0);
			push2lua(l,t1);
			push2lua(l,t2);
			if (lua_pcall(l,4,1,0))
			{
				assert(0);
			}
			int nresults = lua_gettop(l) - level;
			assert(nresults <= 1);
			called_ = false;
			return details::get_return_value<R>()(l,2);
			R r;
			pull2cpp(l,r);
			lua_pop(l,2);//number of arguments ?
			return r;
		}
		friend void pull2cpp<T>(lua_State* const,cpp_acquire_base_ptr<T>& );
	public:
	private:
		bool is_lua_bind_function(lua_State* L,int index)
		{
			if(!lua_getupvalue(L,index,1))
				return false;
			lua_pop(L,1);
			return lua_type(L,-1) == LUA_TFUNCTION;
		}
	};
	//pulls a pointer from the stack which Cpp will then own and call delete on
	template<typename T>
	inline void pull2cpp(lua_State* const s, cpp_acquire_base_ptr<T>&  value)
	{
		assert(s);
		BOOST_STATIC_ASSERT((boost::is_base_and_derived<cpp_acquire_base_ptr<T>,T>::value));
		typename cpp_acquire_base_ptr<T>::raw* class_ptr;
		INTERNAL::pull_class_type<typename cpp_acquire_base_ptr<T>::raw>(s,OOLUA::param_type<T*>::is_constant,class_ptr);
		if(!class_ptr )
		{
#if 1 
			INTERNAL::pull_class_type_error(s,OOLUA::param_type<T*>::is_constant
				? Proxy_class<typename cpp_acquire_base_ptr<T>::raw>::class_name_const 
				: Proxy_class<typename cpp_acquire_base_ptr<T>::raw>::class_name);
#elif defined OOLUA_EXCEPTIONS
			throw Type_error("tried to pull a type that is not the type or a base of the type on the stack");
#else
			value.m_ptr = 0;
#endif
			return;
		}
		assert(class_ptr);
		value.m_ptr = class_ptr;


		INTERNAL::Lua_ud* ud = INTERNAL::find_ud_dont_care_about_type_and_clean_stack(s,value.m_ptr);
		value.ud = ud->ud;
		//lua_pushlightuserdata(s,class_ptr);
		//lua_rawget(s,wt);
		//assert(! lua_isnil(s,-1));
		value.called_ = false;
		//value.m_ptr->SetScriptObject(value);		
		static_cast<cpp_acquire_base_ptr<T>&>(*value.m_ptr) = value;
		INTERNAL::set_owner(s,value.m_ptr,OOLUA::Cpp);
		lua_pop( s, 1);
	}

#define OOLUA_MAKE_SCRIPT_DERIVED(SD)\
	private:\
	SD(){}\
	friend class OOLUA::cpp_acquire_base_ptr<SD>;\
	friend int OOLUA::INTERNAL::create_type<SD>(lua_State * );\




}


#include <boost/preprocessor/facilities/expand.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>

#define __VFUNC_PROCESS_ARG(R,PREFIX,I,ARG)\
	BOOST_PP_COMMA_IF(I)\
	BOOST_PP_EXPAND(\
			BOOST_PP_CAT(BOOST_PP_CAT(__VFUNC_,PREFIX),ARG)\
		       )\

#define __VFUNC_PROCESS_ARGS(PREFIX,ARGS)\
	BOOST_PP_SEQ_FOR_EACH_I(\
			__VFUNC_PROCESS_ARG,\
			PREFIX,\
			ARGS\
			)\


#define VFUNC_TO_CPP_IMPL(FN)\
	_##FN##_cpp_default_impl_

#define __VFUNC_FUNCTION(BASE,CLASS,RT,FN,ARGS,_IS_CONST)\
	RT FN(__VFUNC_PROCESS_ARGS(DECL_,ARGS))_IS_CONST\
	{\
			return call_lua_func<RT>(#FN,__VFUNC_PROCESS_ARGS(CALL_,ARGS),&CLASS::VFUNC_TO_CPP_IMPL(FN) ); \
	}	\
	RT VFUNC_TO_CPP_IMPL(FN) (__VFUNC_PROCESS_ARGS(DECL_,ARGS))_IS_CONST\
	{\
			return BASE::FN(__VFUNC_PROCESS_ARGS(CALL_,ARGS));\
	}\


#include <boost/preprocessor/tuple/to_seq.hpp>
	
#define VFUNC_METHOD(BASE,CLASS,RET,FN,AN,AL)\
	__VFUNC_FUNCTION(BASE,CLASS,\
			RET,\
			FN,\
			BOOST_PP_TUPLE_TO_SEQ(AN,AL),\
			\
		       )\

#define VFUNC_CONST_METHOD(BASE,CLASS,RET,FN,AN,AL)\
	__VFUNC_FUNCTION(BASE,CLASS,\
			RET,\
			FN,\
			BOOST_PP_TUPLE_TO_SEQ(AN,AL),\
			const\
		       )\

#define  VFUNC_CONST_VOID_METHOD(BASE,CLASS,FN,AN,AL)\
	VFUNC_CONST_METHOD(BASE,CLASS,void,FN,AN,AL)\

#define __VFUNC_DECL_I_(TYPE,NAME) TYPE NAME

#define __VFUNC_CALL_I_(TYPE,NAME) NAME



#endif
