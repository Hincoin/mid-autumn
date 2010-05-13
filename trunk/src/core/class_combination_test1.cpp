
#include "TypeMap.hpp"


#include "ParamSet.hpp"
#include <string>
using namespace std;

using namespace ma;
struct test{virtual ~test(){}};
typedef test* (creator_fun_t)(const ParamSet&);
typedef test* (*creator_fun_ptr)(const ParamSet&);
type_name2code name_to_code;
typedef boost::variant<creator_fun_ptr> creator_function_type;
boost::unordered_map<type_code_comp_t,creator_function_type> test_makers;
template<typename T0,typename T1>
struct test2:test{};
template<typename T> struct sample_creator;
template<typename T0,typename T1>
struct sample_creator<test2<T0,T1> >{
static test* create(const ParamSet&){return new test2<T0,T1>();}

};
template<typename T>
struct test3{};
MAKE_TYPE_STR_MAP(0,char,integer,name_to_code);
MAKE_TYPE_STR_MAP(1,test3,test3,name_to_code);

REGISTER_TYPE_STR_MAKER_MAP(2,test2,test2_type,name_to_code,sample_creator,test_makers);

#include "simple_test_framework.hpp"

namespace {
	bool test_me(){
		const size_t* code = map_type_str<test2<char,test3<char> > >::type_code();
		size_t code_size = map_type_str<test2<char,test3<char> > >::type_code_size_;
		type_code_comp_t codes(code,code+code_size);
		for(type_name2code::iterator it = name_to_code.begin();
				it != name_to_code.end();++it)
		{
			printf("%s->%d \t",it->first.c_str(),it->second);
		}
		creator_fun_ptr f = ma::get_creator<creator_fun_ptr>(codes,test_makers);
		printf("%p\n",f);

		type_code_comp_t codes_test;
		codes_test.push_back(name_to_code["integer"]);
		codes_test.push_back(name_to_code["integer"]);
		codes_test.push_back(name_to_code["test3"]);
		codes_test.push_back(name_to_code["test2_type"]);
		creator_fun_ptr f_test = ma::get_creator<creator_fun_ptr>(codes_test,test_makers);
		test* t = f_test(ParamSet());
		assert(f == f_test);
		
		return true;
	}
	REGISTER_TEST_FUNC(test_me,test_me)
}

namespace ma{

}
