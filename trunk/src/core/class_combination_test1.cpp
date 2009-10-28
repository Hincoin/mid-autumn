#include <boost/preprocessor/facilities/expand.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>

template<typename T> struct map_type_str;

//template<typename T0,typename T1>
//sample_creator<geometry_primitive<T0,T1> >{
//static primitive* create(const ParamSet& p);
//};
//REGISTER_TYPE_STR_MAKER_MAP(2,geometry_primitive,sample_creator)

#define PP_ADD() +
#define PP_ADD_IF(cond) BOOST_PP_IF(cond, PP_ADD,BOOST_PP_EMPTY)()

#define PP_ADD_TYPE_STR_SIZE(z, N, _)\
	PP_ADD_IF(N) map_type_str<T##N>::type_str_size

#define PP_STR_CPY(z, N, _)\
	static const size_t _offset_##N = BOOST_PP_REPEAT(N,PP_ADD_TYPE_STR_SIZE,_) - N;\
	::strcpy(type_str_ + _offset_##N,map_type_str<T##N>::type_str());\

#define MAKE_SIMPLE_TYPE_STR(N,TYPE,TYPE_STR)\
	template<>\
	struct map_type_str<TYPE>{\
	static const char* type_str(){return #TYPE_STR;}\
	static const size_t type_str_size = sizeof(#TYPE_STR);\
	typedef TYPE type;\
	};\
       	
#define MAKE_TML_TYPE_STR(N,TMLCLASS,TMLCLASSNAME)\
	template<BOOST_PP_ENUM_PARAMS(N,typename T)>\
struct map_type_str<TMLCLASS<BOOST_PP_ENUM_PARAMS(N,T)> >{\
	typedef map_type_str<TMLCLASS<BOOST_PP_ENUM_PARAMS(N,T)> > type;\
	static const size_t type_str_size = BOOST_PP_REPEAT(N,PP_ADD_TYPE_STR_SIZE,_) - N + sizeof( #TMLCLASSNAME );\
	char type_str_[type_str_size];\
	map_type_str(){\
	::memset(type_str_,0,type_str_size);\
	BOOST_PP_REPEAT(N,PP_STR_CPY,_)		\
	::strcpy(type_str_+type_str_size-sizeof( #TMLCLASSNAME ),#TMLCLASSNAME);\
}\
	static const char* type_str(){\
	static type self;\
	return self.type_str_;\
}\
};\


#define PP_MAKE_TYPE_STR_MAP(N)\
	BOOST_PP_IF(N, MAKE_TML_TYPE_STR,MAKE_SIMPLE_TYPE_STR)\

#define MAKE_TYPE_STR_MAP(N,TMLCLASS,TMLCLASSNAME)\
	PP_MAKE_TYPE_STR_MAP(N)(N,TMLCLASS,TMLCLASSNAME) 


#define REGISTER_TYPE_STR_MAKER_MAP(N,TMLCLASS,TMLCLASSNAME,CREATOR,MAKERMAP)\
template<BOOST_PP_ENUM_PARAMS(N,typename T)>\
struct map_type_str<TMLCLASS<BOOST_PP_ENUM_PARAMS(N,T)> >{\
	typedef map_type_str<TMLCLASS<BOOST_PP_ENUM_PARAMS(N,T)> > type;\
	static const size_t type_str_size = BOOST_PP_REPEAT(N,PP_ADD_TYPE_STR_SIZE,_) - N + sizeof( #TMLCLASSNAME );\
	char type_str_[type_str_size];\
	map_type_str(){\
		::memset(type_str_,0,type_str_size);\
		BOOST_PP_REPEAT(N,PP_STR_CPY,_)		\
		::strcpy(type_str_+type_str_size-sizeof( #TMLCLASSNAME ), #TMLCLASSNAME );\
		MAKERMAP.insert(std::make_pair(type_str_,&CREATOR<TMLCLASS<BOOST_PP_ENUM_PARAMS(N,T)> >::create));\
	}\
	static const char* type_str(){\
		static type self;\
		return self.type_str_;\
	}\
};\


#include "ParamSet.hpp"
#include <boost/unordered_map.hpp>
#include <string>
using namespace std;


struct test{virtual ~test(){}};
typedef test* (*creator_fun_ptr)(const ParamSet&);
boost::unordered_map<string,creator_fun_ptr> test_makers;
template<typename T0,typename T1>
struct test2:test{};
template<typename T> struct sample_creator;
template<typename T0,typename T1>
struct sample_creator<test2<T0,T1> >{
static test* create(const ParamSet&){return new test2<T0,T1>();}

};
template<typename T>
struct test3{};
MAKE_TYPE_STR_MAP(0,int,integer);
MAKE_TYPE_STR_MAP(1,test3,test3);

REGISTER_TYPE_STR_MAKER_MAP(2,test2,test2_type,sample_creator,test_makers);

#include "simple_test_framework.hpp"

namespace {
	bool test_me(){
		printf("%s\n",map_type_str<test2<int,test3<int> > >::type_str());
		return true;
	}
}

