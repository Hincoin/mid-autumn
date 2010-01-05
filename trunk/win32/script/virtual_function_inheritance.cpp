
#include "oolua.h"
namespace OOLUA{
	
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
	

	template<typename T>
	struct cpp_acquire_base_ptr{
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

		cpp_acquire_base_ptr(raw* ptr):m_ptr(ptr),m_lua(0),ref(LUA_NOREF),called_(false){}
		cpp_acquire_base_ptr():m_ptr(0),m_lua(0),ref(LUA_NOREF),called_(false){}

		raw* m_ptr;
	protected:
		lua_State* m_lua;
		int ref;
		bool called_;

		template<typename R,typename T0,typename T1,typename T2>
		R call(const char* func_name,T0 t0,T1 t1,T2 t2,R (T::*f)(T0,T1,T2))//pass default call
		{
			assert(ref != LUA_NOREF);
			assert(m_lua);

			if (called_)
			{
				called_=false;
				//default call
				return (m_ptr->*f)(t0,t1,t2);
			}
			called_ = true;
			lua_getref(m_lua,ref);

			printf("%d,",lua_type(m_lua,-1) );
			OOLUA::push2lua(m_lua,"__index");//table key
			lua_gettable(m_lua, -2);//table value
			printf("%d,",lua_type(m_lua,-1) );
			lua_pushstring(m_lua,func_name);
			lua_gettable(m_lua,-2);
			
			bool is_func = lua_type(m_lua,-1) == LUA_TFUNCTION;
			assert(is_func);

			INTERNAL::Lua_ud* ud = INTERNAL::find_ud_dont_care_about_type_and_clean_stack(m_lua,m_ptr);
			lua_pushlightuserdata(m_lua,ud);
			push2lua(m_lua,t0);
			assert(lua_type(m_lua,-1) == LUA_TNUMBER);
			
			push2lua(m_lua,t1);
			assert(lua_type(m_lua,-1) == LUA_TNUMBER);
			push2lua(m_lua,t2);
			assert(lua_type(m_lua,-1) == LUA_TNUMBER);
			int a = lua_isnumber(m_lua,-1);
			int b = lua_isnumber(m_lua,-2);
			int c = lua_isnumber(m_lua,-3);
			if (lua_pcall(m_lua,4,1,0))
			{
				assert(0);
			}
			called_ = false;
			R r;
			pull2cpp(m_lua,r);
			return r;
		}
		 friend void pull2cpp<T>(lua_State* const,cpp_acquire_base_ptr<T>& );
	public:
		cpp_acquire_base_ptr(const cpp_acquire_base_ptr& other)
			:m_lua(other.m_lua),called_(called_),m_ptr(other.m_ptr)
		{
			lua_getref(m_lua,other.ref);
			ref = lua_ref(m_lua,-1);
		}
		void swap(cpp_acquire_base_ptr& other)
		{
			std::swap(m_lua,other.m_lua);
			std::swap(ref,other.ref);
			std::swap(called_,other.called_);
			std::swap(m_ptr,other.m_ptr);
		};
		cpp_acquire_base_ptr& operator=(cpp_acquire_base_ptr other)
		{
			swap(other);
			return *this;
		}
		 ~cpp_acquire_base_ptr(){
			 if (ref != LUA_NOREF)
				lua_unref(m_lua,ref);
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
		value.ref = ud->ref;
		//lua_pushlightuserdata(s,class_ptr);
		//lua_rawget(s,wt);
		//assert(! lua_isnil(s,-1));
		value.m_lua = s;
		
		//value.m_ptr->SetScriptObject(value);		
		static_cast<cpp_acquire_base_ptr<T>&>(*value.m_ptr) = value;
		INTERNAL::set_owner(s,value.m_ptr,OOLUA::Cpp);
		lua_pop( s, 1);
	}
}
struct AbstractClass{
	virtual ~AbstractClass(){printf("~abstract destructor\n");}

	virtual int func0(float x,float y,float z) = 0;
};

//cpp object
struct Derived:public AbstractClass{
	~Derived(){printf("Derived::destructor \n");}
	int func0(float x,float y,float z)
	{ printf("call cpp func0(%.3f,%.3f,%.3f)\n",x,y,z);return -1;} //default implementation
};
//lua object
struct ScriptDerived:Derived,public OOLUA::cpp_acquire_base_ptr<ScriptDerived>{
	//OOLUA::cpp_acquire_base_ptr<ScriptDerived> self_;
	//void SetScriptObject(OOLUA::cpp_acquire_base_ptr<ScriptDerived> s){self_ = s;};
	friend class OOLUA::cpp_acquire_base_ptr<ScriptDerived>;
	friend int OOLUA::INTERNAL::create_type<ScriptDerived>(lua_State * );
private:
	
	ScriptDerived(){}
public:
	int func0_call_from_script(float x,float y,float z){
		return this->Derived::func0(x,y,z);
	}
	int func0(float x,float y,float z){
		//script implementation
		return call<int>("func0",x,y,z,&ScriptDerived::func0_call_from_script);
	}
};

MAKE_CPP_PORXY(Derived,ScriptDerived)


LUA_PROXY_CLASS(AbstractClass)
OOLUA_TYPEDEFS Abstract OOLUA_END_TYPES
LUA_MEM_FUNC(int(float,float,float),func0)
LUA_PROXY_CLASS_END

EXPORT_OOLUA_FUNCTIONS_1_NON_CONST(AbstractClass,func0);
EXPORT_OOLUA_FUNCTIONS_0_CONST(AbstractClass)

///////////////////////////////////////////////////////////////////////////

LUA_PROXY_CLASS(Derived,AbstractClass)
OOLUA_NO_TYPEDEFS
LUA_MEM_FUNC(int(float,float,float),func0)
LUA_PROXY_CLASS_END

EXPORT_OOLUA_FUNCTIONS_0_CONST(Derived)
EXPORT_OOLUA_FUNCTIONS_1_NON_CONST(Derived,func0)

//////////////////////////////////////////////////////////////////////////

LUA_PROXY_CLASS(ScriptDerived,Derived)
OOLUA_NO_TYPEDEFS
LUA_MEM_FUNC_RENAME(int(float,float,float),func0,func0_call_from_script)
LUA_PROXY_CLASS_END

EXPORT_OOLUA_FUNCTIONS_0_CONST(ScriptDerived)

char const OOLUA::Proxy_class< ScriptDerived >::class_name[] = "ScriptDerived";
int const OOLUA::Proxy_class< ScriptDerived >::name_size = sizeof("ScriptDerived")-1; 
OOLUA::Proxy_class< ScriptDerived >::Reg_type OOLUA::Proxy_class< ScriptDerived >::class_methods[]={
	{"func0",&Proxy_class< ScriptDerived >::func0},
	{0,0}
};

void test_virtual_inheritance()
{
		OOLUA::Script* lua = new OOLUA::Script;
		lua->register_class<ScriptDerived>(); //--
		const char* trunk =	"function ScriptDerived:func0(x,y,z) print(\"lua \",x,y,z);return 1000; end  function func() local x = ScriptDerived:new();x:func0(11,22,33); return x; end";
		if(lua->run_chunk(trunk))
		{
			lua->call("func");
			//OOLUA::cpp_acquire_base_ptr<ScriptDerived> r; 
			OOLUA::make_cpp_proxy<Derived>::type r;
			OOLUA::pull2cpp(*lua,r);
			AbstractClass* abc = r.m_ptr;
			lua->gc();
			int x = abc->func0(1.1f,2.2f,3.3f);
			printf("func0 return : %d \n",x);
		}
		delete lua;
}

