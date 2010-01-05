#	include "oolua.h"
#	include "common_cppunit_headers.h"

namespace
{
	class A
	{
	public:
		int a(){return 1;}
	};
	class B
	{
	public:
		int b(){return 2;}
	};
}


LUA_PROXY_CLASS(A)
	OOLUA_NO_TYPEDEFS
	LUA_MEM_FUNC(int(),a)
LUA_PROXY_CLASS_END

EXPORT_OOLUA_FUNCTIONS_1_NON_CONST(A ,a)
EXPORT_OOLUA_FUNCTIONS_0_CONST(A)

LUA_PROXY_CLASS(B)
	OOLUA_NO_TYPEDEFS
	LUA_MEM_FUNC(int(),b)
LUA_PROXY_CLASS_END

EXPORT_OOLUA_FUNCTIONS_1_NON_CONST(B,b)
EXPORT_OOLUA_FUNCTIONS_0_CONST(B)



class UserData : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(UserData);
		CPPUNIT_TEST(cppMethodCall_passTwoInstancesTwice_noException);
	CPPUNIT_TEST_SUITE_END();

	OOLUA::Script * m_lua;
public:
    UserData():m_lua(0){}
    LVD_NOCOPY(UserData)
	void setUp()
	{
		m_lua = new OOLUA::Script;
		m_lua->register_class<A>();
		m_lua->register_class<B>();
	}
	void tearDown()
	{
		delete m_lua;
	}

	void cppMethodCall_passTwoInstancesTwice_noException()
	{
		m_lua->run_chunk(\
				"foo = function(o1,o2,o3,o4)\n"
					"o3:a()\n"
					"o4:b()\n"
				"end");
		A a;
		B b;
		CPPUNIT_ASSERT_NO_THROW( m_lua->call("foo",&a,&b,&a,&b) );
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION( UserData );
