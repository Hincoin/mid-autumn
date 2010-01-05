#	include "oolua.h"
#	include "common_cppunit_headers.h"



struct Lua2Cpp1{};
struct Lua2Cpp2{};

LUA_PROXY_CLASS(Lua2Cpp1)
	OOLUA_NO_TYPEDEFS
LUA_PROXY_CLASS_END
EXPORT_OOLUA_NO_FUNCTIONS(Lua2Cpp1)

LUA_PROXY_CLASS(Lua2Cpp2)
	OOLUA_NO_TYPEDEFS
LUA_PROXY_CLASS_END
EXPORT_OOLUA_NO_FUNCTIONS(Lua2Cpp2)


namespace
{
	template<typename T>
	std::string stringify(T const& t)
	{
		std::stringstream ss;
		ss <<t;
		return ss.str();
	}
	std::string functionReturnsInputs(OOLUA::Script * lua,int input_count)
	{
		std::string inputs;
		std::string input_prefix("i");
		for(int i = 1; i<=input_count;++i)
		{
			inputs+= input_prefix + stringify(i);
			if(i<input_count)inputs+= std::string(", ");
		}
		lua->run_chunk(
			std::string("foo = function(") + inputs + std::string(") ")
			+ std::string("return ") + inputs
			+ std::string(" end") );
		return "foo";
	}
	template<typename INPUT>
	INPUT pull_back_one_input(OOLUA::Script * lua,INPUT const& input)
	{
		std::string func_name = functionReturnsInputs(lua,1);
		INPUT result(0);
		lua->call(func_name,input);
		OOLUA::pull2cpp(*lua,result);
		return result;
	}
}
class Exchange_lua2cpp : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(Exchange_lua2cpp);
	CPPUNIT_TEST(pullSignedInt_functionReturnsInput_resultEqualToInput);
	CPPUNIT_TEST(pullSignedChar_functionReturnsInput_resultEqualToInput);
	CPPUNIT_TEST(pullSignedShort_functionReturnsInput_resultEqualToInput);
	CPPUNIT_TEST(pullUnsignedInt_functionReturnsInput_resultEqualToInput);
	CPPUNIT_TEST(pullUnsignedChar_functionReturnsInput_resultEqualToInput);
	CPPUNIT_TEST(pullUnsignedShort_functionReturnsInput_resultEqualToInput);
	CPPUNIT_TEST(pullPlainChar_functionReturnsInput_resultEqualToInput);
	CPPUNIT_TEST(pullFloat_functionReturnsInput_resultEqualToInput);
	CPPUNIT_TEST(pullDouble_functionReturnsInput_resultEqualToInput);
	CPPUNIT_TEST(pullInts_functionReturnsTheTwoIntsInputted_resultsEqualToInput);
	CPPUNIT_TEST(pullClass_functionReturnsInput_addressComparesEqualToInput);
	CPPUNIT_TEST(pullClasses_functionReturnsTheTwoDifferentInputtedClasses_pointersComparesEqualToInputs);
	CPPUNIT_TEST(pullTable_luaTableOnStack_tableIsValid);
	CPPUNIT_TEST(pullLuaFunction_luaFunctionOnStack_functionIsValid);
	CPPUNIT_TEST_SUITE_END();

	OOLUA::Script * m_lua;
public:
    Exchange_lua2cpp():m_lua(0){}
    LVD_NOCOPY(Exchange_lua2cpp)
	void setUp()
	{
		m_lua = new OOLUA::Script;
	}
	void tearDown()
	{
		delete m_lua;
	}

	void pullSignedInt_functionReturnsInput_resultEqualToInput()
	{
		signed int input = -55;
		signed int result = pull_back_one_input(m_lua,input);
		CPPUNIT_ASSERT_EQUAL(input, result);

	}
	void pullSignedChar_functionReturnsInput_resultEqualToInput()
	{
		signed char input (-1);
		signed char result = pull_back_one_input(m_lua,input);
		CPPUNIT_ASSERT_EQUAL(input, result);
	}
	void pullSignedShort_functionReturnsInput_resultEqualToInput()
	{
		signed short input (-255);
		signed short result = pull_back_one_input(m_lua,input);
		CPPUNIT_ASSERT_EQUAL(input, result);
	}
	void pullUnsignedInt_functionReturnsInput_resultEqualToInput()
	{
		unsigned int input = 5;
		unsigned int result = pull_back_one_input(m_lua,input);
		CPPUNIT_ASSERT_EQUAL(input, result);
	}
	void pullUnsignedChar_functionReturnsInput_resultEqualToInput()
	{
		unsigned char input (1);
		unsigned char result = pull_back_one_input(m_lua,input);
		CPPUNIT_ASSERT_EQUAL(input, result);
	}
	void pullUnsignedShort_functionReturnsInput_resultEqualToInput()
	{
		unsigned short input (255);
		unsigned short result = pull_back_one_input(m_lua,input);
		CPPUNIT_ASSERT_EQUAL(input, result);
	}
	void pullPlainChar_functionReturnsInput_resultEqualToInput()
	{
		char input (120);
		char result = pull_back_one_input(m_lua,input);
		CPPUNIT_ASSERT_EQUAL(input, result);
	}
	void pullFloat_functionReturnsInput_resultEqualToInput()
	{
		float input (51.1234f);
		float  result = pull_back_one_input(m_lua,input);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(input, result,0.1f);
	}
	void pullDouble_functionReturnsInput_resultEqualToInput()
	{
		double input (5551.1234);
		double  result = pull_back_one_input(m_lua,input);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(input, result,0.1);
	}

	void pullClass_functionReturnsInput_addressComparesEqualToInput()
	{
		m_lua->register_class<Lua2Cpp1>();
		std::string func_name = functionReturnsInputs(m_lua,1);
		Lua2Cpp1 input;
		Lua2Cpp1* result(0);
		m_lua->call(func_name,&input);
		OOLUA::pull2cpp(*m_lua,result);
		CPPUNIT_ASSERT_EQUAL(&input, result);
	}
	void pullClasses_functionReturnsTheTwoDifferentInputtedClasses_pointersComparesEqualToInputs()
	{
		m_lua->register_class<Lua2Cpp1>();
		m_lua->register_class<Lua2Cpp2>();
		std::string func_name = functionReturnsInputs(m_lua,2);
		Lua2Cpp1 input1;
		Lua2Cpp1* result1(0);
		Lua2Cpp2 input2;
		Lua2Cpp2* result2(0);

		m_lua->call(func_name,&input1,&input2);

		OOLUA::pull2cpp(*m_lua,result2);
		OOLUA::pull2cpp(*m_lua,result1);
		CPPUNIT_ASSERT_EQUAL(&input2, result2);
		CPPUNIT_ASSERT_EQUAL(&input1, result1);

	}
	void pullInts_functionReturnsTheTwoIntsInputted_resultsEqualToInput()
	{
		std::string func_name = functionReturnsInputs(m_lua,2);
		int input1 = 54;
		int input2 = 2;
		int result1(0);
		int result2(0);

		m_lua->call(func_name,input1,input2);
		OOLUA::pull2cpp(*m_lua,result2);
		CPPUNIT_ASSERT_EQUAL_MESSAGE("input2 is incorrect",input2, result2);
		OOLUA::pull2cpp(*m_lua,result1);
		CPPUNIT_ASSERT_EQUAL_MESSAGE("input1 is incorrect",input1, result1);
	}

	void pullTable_luaTableOnStack_tableIsValid()
	{
		OOLUA::Lua_table table;
		m_lua->run_chunk(
			"func = function() "
				"local t={} "
				"return t "
			"end");
		m_lua->call("func");
		OOLUA::pull2cpp(*m_lua,table);
		CPPUNIT_ASSERT_EQUAL(true, table.valid() );
	}
	void pullLuaFunction_luaFunctionOnStack_functionIsValid()
	{
		OOLUA::Lua_func_ref lua_func;
		m_lua->run_chunk(
			"bar = function() end "
			"func = function() "
				"return bar "
			"end");
		m_lua->call("func");
		OOLUA::pull2cpp(*m_lua,lua_func);
		CPPUNIT_ASSERT_EQUAL(true, lua_func.valid() );
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION( Exchange_lua2cpp );
