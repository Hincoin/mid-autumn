#include <string>
#include <iostream>
#include "oolua.h"

//#include "proxy_class_registration.hpp"
struct foo
{
	foo(){}
	foo(int a){
		printf("foo::foo %d\n",a);
	}
	int bar(float x,float y,float z){printf("cpp foo:bar %f,%f,%f\n ",x,y,z);return int(x);}
	foo* make_foo(){printf("cpp make_foo\n");return new foo(10);}
	int bar2(float,int){}
	~foo(){
		printf("foo::~foo\n");
	}
};

LUA_PROXY_CLASS(foo)
OOLUA_NO_TYPEDEFS
LUA_MEM_FUNC(int(float,float,float),bar)
LUA_MEM_FUNC(int(float,int),bar2)
LUA_MEM_FUNC(foo*(),make_foo)
LUA_PROXY_CLASS_END

EXPORT_OOLUA_FUNCTIONS_2_NON_CONST(foo,bar,make_foo)
EXPORT_OOLUA_FUNCTIONS_0_CONST(foo)

//////////////////////////////////////////////////////////////////////////
struct virtual_foo{
	virtual int bar(float) = 0;
	virtual ~virtual_foo(){}
};
LUA_PROXY_CLASS(virtual_foo)
OOLUA_TYPEDEFS Abstract OOLUA_END_TYPES
LUA_MEM_FUNC(int(float),bar)
LUA_PROXY_CLASS_END

EXPORT_OOLUA_FUNCTIONS_1_NON_CONST(virtual_foo,bar);
EXPORT_OOLUA_FUNCTIONS_0_CONST(virtual_foo)

struct baz : virtual_foo
{
	int bar(float){return 1;}
	int bar_const(float)const{return 2;}
};

LUA_PROXY_CLASS(baz,virtual_foo)
OOLUA_NO_TYPEDEFS
LUA_MEM_FUNC_CONST(int(float),bar_const)
LUA_PROXY_CLASS_END

EXPORT_OOLUA_FUNCTIONS_0_NON_CONST(baz)
EXPORT_OOLUA_FUNCTIONS_1_CONST(baz,bar_const)

struct A{};
struct B:A{};
struct C{};
struct D:B,C{};
using namespace OOLUA;

LUA_PROXY_CLASS(A )
OOLUA_NO_TYPEDEFS
LUA_PROXY_CLASS_END
LUA_PROXY_CLASS(B,A )
OOLUA_NO_TYPEDEFS
LUA_PROXY_CLASS_END
LUA_PROXY_CLASS(C)
OOLUA_NO_TYPEDEFS
LUA_PROXY_CLASS_END
LUA_PROXY_CLASS(D,B,C )
OOLUA_NO_TYPEDEFS
LUA_PROXY_CLASS_END

EXPORT_OOLUA_NO_FUNCTIONS(A)
EXPORT_OOLUA_NO_FUNCTIONS(B)
EXPORT_OOLUA_NO_FUNCTIONS(C)
EXPORT_OOLUA_NO_FUNCTIONS(D)

BOOST_STATIC_ASSERT(!(mpl::empty<mpl::vector<A,B> >::value));

OOLUA::Proxy_class<D> x;
typedef OOLUA::Proxy_class<D>::AllBases all_bases;
using namespace std;
void test_virtual_inheritance();
int main()
{
	test_virtual_inheritance();
	OOLUA::Script* lua = new OOLUA::Script;
	lua->register_class<foo>();
	lua->register_class<virtual_foo>();
	lua->register_class<baz>();
	lua->register_class<A>();
	lua->register_class<B>();
	lua->register_class<C>();
	lua->register_class<D>();

	bool bok = true;
	if(lua->run_chunk("\
					  function foo:make_foo()print(\"lua make_foo\") return foo:new() end;\
					  g_test = function () local f = foo:new(1); print(\"foo::bar \",f:bar(1,2,3)); print(\"a test function\") return f:make_foo();end"))
	{
		lua->call("g_test");
		
		foo* result;
		OOLUA::pull2cpp(*lua,result);
		result->bar(10,20,30);
		lua->gc();
	}
	else bok = false;

	//virtual function
	if (lua->run_chunk("function baz:bar(x) print(\"lua baz:bar()\",x) return 10; end;\
					   virtual_test = function() local f = baz:new(); return f end;"))
	{
		lua->call("virtual_test");
		baz* b;
		OOLUA::cpp_acquire_ptr<virtual_foo*> r; 
		OOLUA::pull2cpp(*lua,r);
		r.m_ptr->bar(100.f);
	}
	if(! bok)
		printf("test failed!\n");
	delete lua;
}
