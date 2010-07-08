#include "oolua.h"
#include <iostream>

using namespace OOLUA;

class AbstractBase{
		public:
	virtual int foo(float ,char,int)=0;
	virtual void bar(float)=0;
	virtual ~AbstractBase(){}
};

class DefaultDerivedBase:public AbstractBase
{
		public:
		virtual int foo(float a,char b,int c){
				std::cout<<"default function foo called: "<<a<<" " << b << " "<< std::endl;
				return 100;
		}
		virtual void bar(float f)
		{
				std::cout<<"default bar called: "<<f<<std::endl;
		}
};

class ScriptDerived:public DefaultDerivedBase, public cpp_acquire_base_ptr<ScriptDerived>
{

		typedef DefaultDerivedBase parent_type;
		typedef ScriptDerived class_type;
	friend ScriptDerived* Construtor<ScriptDerived,boost::mpl::vector<> >::call_construct<boost::mpl::vector<> >(lua_State* , boost::fusion::vector0<>&);
	OOLUA_MAKE_SCRIPT_DERIVED(ScriptDerived)

		public:	
		
			VFUNC_METHOD(DefaultDerivedBase,ScriptDerived,int,foo,3,(I_(float,a),I_(char,b),I_(int,c)))
			VFUNC_METHOD(parent_type,class_type,void,bar,1,(I_(float,f)));
};

MAKE_CPP_PORXY(DefaultDerivedBase,ScriptDerived)

LUA_PROXY_CLASS(ScriptDerived)
OOLUA_NO_TYPEDEFS
LUA_CTORS()
LUA_MEM_FUNC_RENAME(int(float,char ,int),foo,VFUNC_TO_CPP_IMPL(foo))
LUA_MEM_FUNC_RENAME(void(float),bar,VFUNC_TO_CPP_IMPL(bar))
LUA_PROXY_CLASS_END

//EXPORT_OOLUA_FUNCTIONS_0_CONST(ScriptDerived)
EXPORT_FUNCTIONS_N_CONST(ScriptDerived,0,());
EXPORT_FUNCTIONS_N_NON_CONST(ScriptDerived,2,(foo,bar));

		/*
char const OOLUA::Proxy_class< ScriptDerived >::class_name[] = "ScriptDerived";
int const OOLUA::Proxy_class< ScriptDerived >::name_size = sizeof("ScriptDerived")-1; 
OOLUA::Proxy_class< ScriptDerived >::Reg_type OOLUA::Proxy_class< ScriptDerived >::class_methods[]={
	{"foo",&Proxy_class< ScriptDerived >::foo},
	{"bar",&Proxy_class< ScriptDerived >::bar},
	{0,0}
};
*/

int main()
{
		OOLUA::Script* lua = new OOLUA::Script;
		lua->register_class<ScriptDerived>(); //--
		const char* trunk =	"function  ScriptDerived:bar(f)print(\"lua: \",f)end function ScriptDerived:foo(x,y,z) print(\"lua \",x,y,z);return 1000; end  function func() local x = ScriptDerived:new();x:foo(11.1,22.2,33.3); return x; end";
		if(lua->run_chunk(trunk))
		{
			int k = 0;
			while(k++ < 100 )
			{
				lua->call("func");
				//OOLUA::cpp_acquire_base_ptr<ScriptDerived> r; 
				OOLUA::make_cpp_proxy<DefaultDerivedBase>::type r;
				OOLUA::pull2cpp(*lua,r);
				AbstractBase* abc = r.m_ptr;
				using namespace OOLUA;
				int i = 0;
				int x;
				
				while (i++ < 100)
				{
					x = abc->foo(1.1f,2.2f,3.3f);
					abc->bar(4.4f);
				}
				lua->gc();
				printf("foo return : %d \n",x);
				
				delete abc;
			}

		}
		delete lua;

	return 0;
}
