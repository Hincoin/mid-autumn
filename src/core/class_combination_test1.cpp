#include <boost/preprocessor/facilities/expand.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>

#include <boost/functional/hash.hpp>

#include <vector>
#include <string>
#include <boost/variant/get.hpp>
#include <string.h>
#include <memory>
#include <boost/unordered_map.hpp>
namespace ma{
template<typename T>
	std::size_t hash_value(const std::vector<T>& v)
			{
			return boost::hash_range(v.begin(),v.end());
			}
typedef size_t type_code_t;
typedef std::vector<type_code_t> type_code_comp_t;
typedef std::string type_name_t;
typedef boost::unordered_map<type_name_t,type_code_t> type_name2code;

//typedef boost::unordered_map<type_code_t,creator_t> type_code2creator;
template<typename CreatorFuncPtr,typename CreatorMap>
	const CreatorFuncPtr get_creator(const type_code_comp_t& type_code,const CreatorMap& creators)
	{
		BOOST_MPL_ASSERT((boost::is_pointer<CreatorFuncPtr>));
		typename CreatorMap::const_iterator it = creators.find(type_code);
		if (it == creators.end()) return 0;
		return boost::get<CreatorFuncPtr>((it->second));
	}
}
template<typename T> struct map_type_str;

//template<typename T0,typename T1>
//sample_creator<geometry_primitive<T0,T1> >{
//static primitive* create(const ParamSet& p);
//};
//REGISTER_TYPE_STR_MAKER_MAP(2,geometry_primitive,sample_creator)

#define PP_ADD() +
#define PP_ADD_IF(cond) BOOST_PP_IF(cond, PP_ADD,BOOST_PP_EMPTY)()

#define PP_ADD_TYPE_STR_SIZE(z, N, _)\
	PP_ADD_IF(N) map_type_str<T##N>::type_code_size_

#define PP_STR_CPY(z, N, _)\
	static const size_t _offset_##N = BOOST_PP_REPEAT(N,PP_ADD_TYPE_STR_SIZE,_) - 0 ;\
	::memcpy(type_code_ + _offset_##N,map_type_str<T##N>::type_code(),\
			sizeof(size_t) * map_type_str<T##N>::type_code_size_);\

#define MAKE_SIMPLE_TYPE_STR(N,TYPE,TYPE_STR,TYPE_MAP)\
	template<>\
	struct map_type_str<TYPE>{\
	map_type_str(){\
		if (TYPE_MAP.find(#TYPE_STR) == TYPE_MAP.end())\
		{\
			type_code_[0] = TYPE_MAP.size();\
			TYPE_MAP.insert(std::make_pair(#TYPE_STR,type_code_[0]));\
		}\
	}\
	static const size_t type_code_size_ = 1;\
	size_t type_code_[type_code_size_];\
	typedef TYPE type;\
	static const size_t* type_code(){static map_type_str<TYPE> self;return self.type_code_;}\
	};\
       	
#define MAKE_TML_TYPE_STR(N,TMLCLASS,TMLCLASSNAME,TYPE_MAP)\
	template<BOOST_PP_ENUM_PARAMS(N,typename T)>\
struct map_type_str<TMLCLASS<BOOST_PP_ENUM_PARAMS(N,T)> >{\
	typedef map_type_str<TMLCLASS<BOOST_PP_ENUM_PARAMS(N,T)> > type;\
	static const size_t type_code_size_ = BOOST_PP_REPEAT(N,PP_ADD_TYPE_STR_SIZE,_) + 1;\
	size_t type_code_[type_code_size_];\
	map_type_str(){\
	::memset(type_code_,0,sizeof(size_t)*type_code_size_);\
	BOOST_PP_REPEAT(N,PP_STR_CPY,_)		\
	if (TYPE_MAP.find(#TMLCLASSNAME) == TYPE_MAP.end()){\
		type_code_[type_code_size_ - 1] = TYPE_MAP.size();\
		TYPE_MAP.insert(std::make_pair(#TMLCLASSNAME,type_code_[type_code_size_ - 1]));\
	}\
}\
	static const size_t* type_code(){\
	static type self;\
	return self.type_code_;\
}\
};\


#define PP_MAKE_TYPE_STR_MAP(N)\
	BOOST_PP_IF(N, MAKE_TML_TYPE_STR,MAKE_SIMPLE_TYPE_STR)\

#define MAKE_TYPE_STR_MAP(N,TMLCLASS,TMLCLASSNAME,TYPE_MAP)\
	PP_MAKE_TYPE_STR_MAP(N)(N,TMLCLASS,TMLCLASSNAME,TYPE_MAP) 


#define REGISTER_TYPE_STR_MAKER_MAP(N,TMLCLASS,TMLCLASSNAME,TYPE_MAP,CREATOR,MAKERMAP)\
template<BOOST_PP_ENUM_PARAMS(N,typename T)>\
struct map_type_str<TMLCLASS<BOOST_PP_ENUM_PARAMS(N,T)> >{\
	typedef map_type_str<TMLCLASS<BOOST_PP_ENUM_PARAMS(N,T)> > type;\
	static const size_t type_code_size_ = BOOST_PP_REPEAT(N,PP_ADD_TYPE_STR_SIZE,_) + 1;\
	size_t type_code_[type_code_size_];\
	map_type_str(){\
		::memset(type_code_,0,type_code_size_* sizeof(size_t));\
		BOOST_PP_REPEAT(N,PP_STR_CPY,_)		\
		if (TYPE_MAP.find(#TMLCLASSNAME) == TYPE_MAP.end()){\
			type_code_[type_code_size_ - 1] = TYPE_MAP.size();\
			TYPE_MAP.insert(std::make_pair(#TMLCLASSNAME,type_code_[type_code_size_ - 1]));\
		}\
		MAKERMAP.insert(std::make_pair(type_code_comp_t(type_code_,type_code_ + type_code_size_),CREATOR<TMLCLASS<BOOST_PP_ENUM_PARAMS(N,T)> >::create));\
	}\
	static const size_t* type_code(){\
		static type self;\
		return self.type_code_;\
	}\
};\


#include "ParamSet.hpp"
#include <string>
using namespace std;

using namespace ma;
struct test{virtual ~test(){}};
typedef test* (creator_fun_t)(const ParamSet&);
typedef test* (*creator_fun_ptr)(const ParamSet&);
type_name2code name2code;
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
MAKE_TYPE_STR_MAP(0,int,integer,name2code);
MAKE_TYPE_STR_MAP(1,test3,test3,name2code);

REGISTER_TYPE_STR_MAKER_MAP(2,test2,test2_type,name2code,sample_creator,test_makers);

#include "simple_test_framework.hpp"

namespace {
	bool test_me(){
		const size_t* code = map_type_str<test2<int,test3<int> > >::type_code();
		size_t code_size = map_type_str<test2<int,test3<int> > >::type_code_size_;
		type_code_comp_t codes(code,code+code_size);
		for(type_name2code::iterator it = name2code.begin();
				it != name2code.end();++it)
		{
			printf("%s->%d \t",it->first.c_str(),it->second);
		}
		creator_fun_ptr f = ma::get_creator<creator_fun_ptr>(codes,test_makers);
		printf("%p\n",f);

		type_code_comp_t codes_test;
		codes_test.push_back(name2code["integer"]);
		codes_test.push_back(name2code["integer"]);
		codes_test.push_back(name2code["test3"]);
		codes_test.push_back(name2code["test2_type"]);
		creator_fun_ptr f_test = ma::get_creator<creator_fun_ptr>(codes_test,test_makers);
		assert(f == f_test);
		
		return true;
	}
	REGISTER_TEST_FUNC(test_me,test_me)
}

namespace ma{

}
