


#       include "oolua.h"
#       include "cpp_class_ops.h"
#       include "lua_class_ops.h"

#       include "common_cppunit_headers.h"

const int int_not_set = 0;
const int int_set = 1;
const bool bool_not_set = false;
const bool bool_set = true;
const std::string string_not_set("not set");
const std::string string_set("hello world");
const char* c_string_not_set = "not set";
const char*  c_string_set= "hello world";

namespace
{
	struct Stub1{};
	struct Stub2{};
	struct InvalidStub{};
}
typedef boost::mpl::vector<> v0_t;
typedef boost::mpl::vector<char*> v1_t;
typedef boost::mpl::vector<std::string> v1s_t;
typedef boost::mpl::vector<char*,std::string> v2_t;
typedef boost::mpl::vector<bool> v1b_t;
typedef boost::mpl::vector<int> v1i_t;
typedef boost::mpl::vector<float> v1f_t;
typedef boost::mpl::vector<bool,int,float> v3bif_t;
typedef boost::mpl::vector<bool,char,int> v3bcf_t;

typedef boost::mpl::vector<v0_t,v1_t> vno_same_type;
BOOST_STATIC_ASSERT((boost::mpl::equal<v1s_t,v1_t,OOLUA::is_same_for_lua<boost::mpl::_1,boost::mpl::_2> >::value));
BOOST_STATIC_ASSERT((boost::mpl::equal<v3bif_t,v3bcf_t,OOLUA::is_same_for_lua<boost::mpl::_1,boost::mpl::_2> >::value));

typedef boost::mpl::vector<v3bif_t,v1_t,v1s_t,v1b_t,v1i_t,v1f_t> v2_has_same_type;

typedef OOLUA::lua_type_comparison<vno_same_type >::type false_same_type;

typedef boost::mpl::begin<vno_same_type>::type vno2_begin;
BOOST_STATIC_ASSERT(!(boost::mpl::equal<boost::mpl::deref<vno2_begin>::type,boost::mpl::deref<boost::mpl::next<vno2_begin>::type>::type >::value));
typedef OOLUA::lua_type_comparison_impl<vno2_begin,boost::mpl::next<vno2_begin>::type,boost::mpl::end<vno_same_type>::type>::type t_has_same_type;

BOOST_STATIC_ASSERT((!t_has_same_type::value));



typedef OOLUA::lua_type_comparison< v2_has_same_type >::type true_same_type;
BOOST_STATIC_ASSERT((true_same_type::value));
class ParamConstructor
{
public:
	ParamConstructor(int i)
		:m_int(i)
		,m_bool(bool_not_set)
		,m_int_ptr(int_not_set)
		,m_string(string_not_set)
		,m_stub1(0)
		,m_stub2(0)
		,m_stub_passed_by_value(bool_not_set){}

	ParamConstructor(bool b)
		:m_int(int_not_set)
		,m_bool(b)
		,m_int_ptr(int_not_set)
		,m_string(string_not_set)
		,m_stub1(0)
		,m_stub2(0)
		,m_stub_passed_by_value(bool_not_set){}


	ParamConstructor(int *  i)
		:m_int(int_not_set)
		,m_bool(bool_not_set)
		,m_int_ptr(*i)
		,m_string(string_not_set)
		,m_stub1(0)
		,m_stub2(0)
		,m_stub_passed_by_value(bool_not_set){}

	ParamConstructor(char const *  str)
		:m_int(int_not_set)
		,m_bool(bool_not_set)
		,m_int_ptr(int_not_set)
		,m_string(str)
		,m_stub1(0)
		,m_stub2(0)
		,m_stub_passed_by_value(bool_not_set){}


	ParamConstructor(int i,bool b)
		:m_int(i)
		,m_bool(b)
		,m_int_ptr(int_not_set)
		,m_string(string_not_set)
		,m_stub1(0)
		,m_stub2(0)
		,m_stub_passed_by_value(bool_not_set){}


	ParamConstructor(Stub1* stub)
		:m_int(int_not_set)
		,m_bool(bool_not_set)
		,m_int_ptr(int_not_set)
		,m_string(string_not_set)
		,m_stub1(stub)
		,m_stub2(0)
		,m_stub_passed_by_value(bool_not_set){}


	ParamConstructor(Stub1* stub1,Stub2 * stub2)
		:m_int(int_not_set)
		,m_bool(bool_not_set)
		,m_int_ptr(int_not_set)
		,m_string(string_not_set)
		,m_stub1(stub1)
		,m_stub2(stub2)
		,m_stub_passed_by_value(bool_not_set){}

	ParamConstructor(Stub2 /*stub2*/)
		:m_int(int_not_set)
		,m_bool(bool_not_set)
		,m_int_ptr(int_not_set)
		,m_string(string_not_set)
		,m_stub1(0)
		,m_stub_passed_by_value(bool_set){}

	int m_int;
	bool m_bool;
	int m_int_ptr;
	std::string m_string;
	Stub1* m_stub1;
	Stub2* m_stub2;
	bool m_stub_passed_by_value;
};



LUA_PROXY_CLASS(ParamConstructor)
OOLUA_NO_TYPEDEFS
LUA_CTORS(
		  LUA_CTOR_ARGS(bool),
		 LUA_CTOR_ARGS(int ),
		 LUA_CTOR_ARGS(char const*),
		 LUA_CTOR_ARGS(int,bool),
		 LUA_CTOR_ARGS(Stub1 *),
		 LUA_CTOR_ARGS(Stub1 *,Stub2*),
		 LUA_CTOR_ARGS(Stub2)
		 )
OOLUA_CLASS_END

EXPORT_OOLUA_NO_FUNCTIONS(ParamConstructor)


OOLUA_CLASS_NO_BASES(Stub1)
OOLUA_NO_TYPEDEFS
LUA_CTORS()
OOLUA_CLASS_END

EXPORT_OOLUA_NO_FUNCTIONS(Stub1)

OOLUA_CLASS_NO_BASES(InvalidStub)
OOLUA_NO_TYPEDEFS
LUA_CTORS()
OOLUA_CLASS_END

EXPORT_OOLUA_NO_FUNCTIONS(InvalidStub)

OOLUA_CLASS_NO_BASES(Stub2)
OOLUA_NO_TYPEDEFS
LUA_CTORS()
OOLUA_CLASS_END

EXPORT_OOLUA_NO_FUNCTIONS(Stub2)

class Construct : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(Construct);
	CPPUNIT_TEST(new_luaCreatesInstanceThenReturnsIt_returnIsNoneNull);
	CPPUNIT_TEST(new_luaCreatesInstance_noException);
	CPPUNIT_TEST(register_noDefaultConstructor_compiles);
	CPPUNIT_TEST(new_CallingIntParamConstructor_runChunkReturnsTrue);
	CPPUNIT_TEST(new_CallingIntParamConstructorPassingInitailisedValue_InstanceHasIntSetToInitailisedValue);
	CPPUNIT_TEST(new_CallingBoolParamConstructor_runChunkReturnsTrue);
	//To OOLUA there is no difference between a function that takes an int, int*, int& or any variation
	//the first registered constructor which matches will be called.
	//CPPUNIT_TEST(new_CallingIntPtrParamConstructor_runChunkReturnsTrue);
	//CPPUNIT_TEST(new_CallingIntPtrParamConstructorPassingInitailisedValue_InstanceHasIntPtrSetToInitailisedValue);
	CPPUNIT_TEST(new_CallingCharConstPtrParamConstructor_runChunkReturnsTrue);
	CPPUNIT_TEST(new_CallingCharConstPtrParamConstructorPassingInitailisedValue_InstanceHasStringSetToInitailisedValue);


	CPPUNIT_TEST(new_twoParamConstructorIntAndBool_runChunkReturnsTrue);
	CPPUNIT_TEST(new_twoParamConstructorIntAndBool_InstanceHasIntValueSet);
	CPPUNIT_TEST(new_twoParamConstructorIntAndBool_InstanceHasBoolValueSet);

	CPPUNIT_TEST(new_twoParamConstructorIntAndBoolPassedStringAsFirstParam_runChunkReturnsFalse);

	CPPUNIT_TEST(new_oneParamConstructorStub1_callReturnsTrue);
	CPPUNIT_TEST(new_oneParamConstructorStub1_instanceMemberIsSet);
	CPPUNIT_TEST(new_oneParamConstructorPassingAnInvalidParam_callReturnsFalse);
	CPPUNIT_TEST(new_twoParamConstructorStub1AndStub2_callReturnsTrue);
	CPPUNIT_TEST(new_twoParamConstructorStub1AndInvalid_callReturnsFalse);
	CPPUNIT_TEST(new_oneParamConstructorStub2ByValue_callReturnsTrue);
	CPPUNIT_TEST(new_oneParamConstructorStub2ByValue_instanceMemberIsSet);
	CPPUNIT_TEST_SUITE_END();

	OOLUA::Script * m_lua;
	std::string register_and_create_one_param_constructor()
	{
		m_lua->register_class<ParamConstructor>();
		m_lua->run_chunk("foo = function(i) return ParamConstructor:new(i) end");
		return "foo";
	}
	std::string register_and_create_two_param_constructor()
	{
		m_lua->register_class<ParamConstructor>();
		m_lua->run_chunk("foo = function(i1,i2) return ParamConstructor:new(i1,i2) end");
		return "foo";
	}
	struct ParamConstructorWrapper
	{
		OOLUA::cpp_acquire_ptr<ParamConstructor> instance;
		std::auto_ptr<ParamConstructor> auto_delete_instance;
		void set_ptr_to_auto_delete()
		{
			auto_delete_instance = std::auto_ptr<ParamConstructor>(instance.m_ptr);
		}
	};
	void pull_ParamWrapper(ParamConstructorWrapper& wrap)
	{
		OOLUA::pull2cpp(*m_lua,wrap.instance);
		wrap.set_ptr_to_auto_delete();
	}
public:
	Construct():m_lua(0){}
	LVD_NOCOPY(Construct)
		void setUp()
	{
		m_lua = new OOLUA::Script;
		m_lua->register_class<Class_ops>();
	}
	void tearDown()
	{
		delete m_lua;
	}


	void new_luaCreatesInstanceThenReturnsIt_returnIsNoneNull()
	{
		createAndReturnClassOps(m_lua);
		OOLUA::cpp_acquire_ptr<Class_ops> res;
		OOLUA::pull2cpp(*m_lua,res);
		CPPUNIT_ASSERT_EQUAL(true, res.m_ptr != 0);
		delete res.m_ptr;
	}
	void new_luaCreatesInstance_noException()
	{
		std::string foo("Class_ops:new()");
		CPPUNIT_ASSERT_NO_THROW( m_lua->run_chunk(foo) );
	}

	void createAndReturnClassOps(OOLUA::Script *lua)
	{
		std::string foo(\
			"createAndReturn = function() \n"
			"return Class_ops:new() \n"
			"end");
		lua->run_chunk(foo);
		CPPUNIT_ASSERT_NO_THROW( lua->call("createAndReturn") );
	}

	void register_noDefaultConstructor_compiles()
	{
		m_lua->register_class<ParamConstructor>();
	}
	void new_CallingIntParamConstructor_runChunkReturnsTrue()
	{
		m_lua->register_class<ParamConstructor>();
		bool result = m_lua->run_chunk("ParamConstructor:new(1)");
		CPPUNIT_ASSERT_EQUAL(true,result);

	}
	void new_CallingIntParamConstructorPassingInitailisedValue_InstanceHasIntSetToInitailisedValue()
	{
		m_lua->call(register_and_create_one_param_constructor(),int_set);
		ParamConstructorWrapper wrap;
		pull_ParamWrapper(wrap);
		CPPUNIT_ASSERT_EQUAL(int_set,wrap.instance.m_ptr->m_int);
	}
	void new_CallingBoolParamConstructor_runChunkReturnsTrue()
	{
		m_lua->register_class<ParamConstructor>();
		bool result = m_lua->run_chunk("ParamConstructor:new(true)");
		CPPUNIT_ASSERT_EQUAL(true,result);

	}

	void new_CallingBoolParamConstructorPassingInitailisedValue_InstanceHasBoolSetToInitailisedValue()
	{
		m_lua->call(register_and_create_one_param_constructor(),bool_set);
		ParamConstructorWrapper wrap;
		pull_ParamWrapper(wrap);
		CPPUNIT_ASSERT_EQUAL(bool_set,wrap.instance.m_ptr->m_bool);
	}

	void new_CallingIntPtrParamConstructor_runChunkReturnsTrue()
	{
		m_lua->register_class<ParamConstructor>();
		bool result = m_lua->run_chunk("ParamConstructor:new(1)");
		CPPUNIT_ASSERT_EQUAL(true,result);

	}
	void new_CallingIntPtrParamConstructorPassingInitailisedValue_InstanceHasIntPtrSetToInitailisedValue()
	{
		m_lua->call(register_and_create_one_param_constructor(),int_set);
		ParamConstructorWrapper wrap;
		pull_ParamWrapper(wrap);
		CPPUNIT_ASSERT_EQUAL(int_set,wrap.instance.m_ptr->m_int_ptr);
	}




	void new_CallingCharConstPtrParamConstructor_runChunkReturnsTrue()
	{
		m_lua->register_class<ParamConstructor>();
		bool result = m_lua->run_chunk("ParamConstructor:new(\"do not care\")");
		CPPUNIT_ASSERT_EQUAL(true,result);
	}
	void new_CallingCharConstPtrParamConstructorPassingInitailisedValue_InstanceHasStringSetToInitailisedValue()
	{
		m_lua->call(register_and_create_one_param_constructor(),c_string_set);
		ParamConstructorWrapper wrap;
		pull_ParamWrapper(wrap);
		CPPUNIT_ASSERT_EQUAL(string_set,wrap.instance.m_ptr->m_string);
	}
	void new_twoParamConstructorIntAndBool_runChunkReturnsTrue()
	{
		m_lua->register_class<ParamConstructor>();
		bool result = m_lua->run_chunk("ParamConstructor:new(1,true)");
		CPPUNIT_ASSERT_EQUAL(true,result);
	}
	void new_twoParamConstructorIntAndBool_InstanceHasIntValueSet()
	{
		m_lua->call(register_and_create_two_param_constructor(),int_set,bool_set);
		ParamConstructorWrapper wrap;
		pull_ParamWrapper(wrap);
		CPPUNIT_ASSERT_EQUAL(int_set,wrap.instance.m_ptr->m_int);
	}
	void new_twoParamConstructorIntAndBool_InstanceHasBoolValueSet()
	{
		m_lua->call(register_and_create_two_param_constructor(),int_set,bool_set);
		ParamConstructorWrapper wrap;
		pull_ParamWrapper(wrap);
		CPPUNIT_ASSERT_EQUAL(bool_set,wrap.instance.m_ptr->m_bool);
	}
	void new_twoParamConstructorIntAndBoolPassedStringAsFirstParam_runChunkReturnsFalse()
	{
		m_lua->register_class<ParamConstructor>();
		bool result = m_lua->run_chunk("ParamConstructor:new(\"dont care\",true)");
		CPPUNIT_ASSERT_EQUAL(false,result);
	}

	void new_oneParamConstructorStub1_callReturnsTrue()
	{
		m_lua->register_class<Stub1>();
		Stub1 stub1;
		bool result = m_lua->call(register_and_create_one_param_constructor(),&stub1);
		CPPUNIT_ASSERT_EQUAL(true,result);
	}
	void new_oneParamConstructorStub1_instanceMemberIsSet()
	{
		m_lua->register_class<Stub1>();
		Stub1 stub;
		m_lua->call(register_and_create_one_param_constructor(),&stub);
		ParamConstructorWrapper wrap;
		pull_ParamWrapper(wrap);
		CPPUNIT_ASSERT_EQUAL(&stub,wrap.instance.m_ptr->m_stub1);
	}

	void new_oneParamConstructorPassingAnInvalidParam_callReturnsFalse()
	{
		m_lua->register_class<InvalidStub>();
		InvalidStub stub;
		bool result = m_lua->call(register_and_create_one_param_constructor(),&stub);
		CPPUNIT_ASSERT_EQUAL(false,result);
	}

	void new_twoParamConstructorStub1AndStub2_callReturnsTrue()
	{
		m_lua->register_class<Stub1>();
		m_lua->register_class<Stub2>();
		Stub1 stub1;
		Stub2 stub2;
		bool result = m_lua->call(register_and_create_two_param_constructor(),&stub1,&stub2);
		CPPUNIT_ASSERT_EQUAL(true,result);
	}

	void new_twoParamConstructorStub1AndInvalid_callReturnsFalse()
	{
		m_lua->register_class<Stub1>();
		m_lua->register_class<InvalidStub>();
		Stub1 stub1;
		InvalidStub stub2;
		bool result = m_lua->call(register_and_create_two_param_constructor(),&stub1,&stub2);
		CPPUNIT_ASSERT_EQUAL(false,result);
	}

	void new_oneParamConstructorStub2ByValue_callReturnsTrue()
	{
		m_lua->register_class<Stub2>();
		Stub2 stub;
		bool result = m_lua->call(register_and_create_one_param_constructor(),&stub);
		CPPUNIT_ASSERT_EQUAL(true,result);
	}

	void new_oneParamConstructorStub2ByValue_instanceMemberIsSet()
	{
		m_lua->register_class<Stub2>();
		Stub2 stub;
		m_lua->call(register_and_create_one_param_constructor(),&stub);
		ParamConstructorWrapper wrap;
		pull_ParamWrapper(wrap);
		CPPUNIT_ASSERT_EQUAL(bool_set,wrap.instance.m_ptr->m_stub_passed_by_value);
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION( Construct );
