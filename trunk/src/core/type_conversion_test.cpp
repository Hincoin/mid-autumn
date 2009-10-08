#include "simple_test_framework.hpp"

bool type_conversion_test();


REGISTER_TEST_FUNC(type_conversion,type_conversion_test)

#include "TypeConversion.hpp"

#include <iostream>
#include <typeinfo>

template<>
struct type_promote<char>
{
	typedef short type;
};
template<>
struct type_promote<short>
{
	typedef int type;
};
template<>
struct type_promote<int>{
	typedef double type;
};

template<>
struct type_promote<float>{
	typedef double type;
};
template<>
struct type_promote<double>{
	typedef double type;
};

template<>
struct type_promote<unsigned char>
{
	typedef unsigned short type;
};
template<>
struct type_promote<unsigned short>
{
	typedef unsigned int type;
};
template<>
struct type_promote<unsigned int>{
	typedef double type;
};

//////////////////////////////////////////////////////////////////////////
template<typename T>
struct cast_type_array;

struct Base1{
	int a ;
	virtual ~Base1(){}
};
struct Base2{
	int b;
	virtual ~Base2(){}
};
struct D:virtual Base1,virtual Base2{
	double c;
};

template<>
struct multiple_promote<D>{
	typedef boost::mpl::vector<D,Base1,Base2> type;
};


template<typename T>
inline void print_type()
{
	std::cout<<typeid(T).name()<<std::endl<<sizeof(T)<<std::endl;
}

inline int* null_func()
{
	int a[100];
	a[0] = 100;
	double tmp = 10 ;
	//BOOST_STATIC_ASSERT((boost::is_convertible<double,unsigned&>::value));
	//unsigned long k = (static_cast<const unsigned long&>(tmp));
	//unsigned long* k = &((unsigned long&)(*tmp));
	return a;
}
inline bool promote_test(){
	typedef promote_to_variant<char>::type int_promote;
	typedef boost::variant<char,short,int,double> char_promote;
	print_type<char_promote>();

	typedef promote_types<char>::type char_promotions;
	typedef biggest_type<char_promotions>::type biggest_promotion;
	std::cout<<typeid(biggest_promotion).name()<<" "<<std::endl<<sizeof(biggest_promotion);

	typedef do_promotions<char_promotions>::type type_set;
	typedef flatten_seq_seq<type_set>::type flattened_types;
	print_type<type_set>();
	print_type<flattened_types>();
	typedef recursive_type_promote<char>::type recursive_type;
	print_type<recursive_type>();

	typedef multiple_promote<char>::type multi_char_promote;
	print_type<multi_char_promote>();

	std::cout<<cast_types<char>::casted_types_info.size()<<std::endl;

	bool result = true;
	double d = 10.1;
	//char d = 10;

	const char cc = cast_types<char>::cast_to<char>(&d);
	result = cc == char(d);
	assert(result);
	int *a=null_func();

	double pptr_c = *(const double*)(cast_types<char>::cast_to(typeid(double),&d));
	a=null_func();
	result = result && pptr_c == d;
	assert(result);
	D ddddd;
	ddddd.a = 1;
	ddddd.b = 7;
	ddddd.c = 2.9;


	const Base1& b_dd = *(const Base1*)cast_types<D>::cast_to(typeid(Base1),&ddddd);
	a = null_func();
	const Base2& b_dd2 = *(const Base2*)cast_types<D>::cast_to(typeid(Base2),&ddddd);
	a = null_func();
	result = result && b_dd.a == ddddd.a && 
		b_dd2.b == ddddd.b;
	assert(result);
	return result;
	//std::cout<<(int)c<<std::endl;
};
bool type_conversion_test(){
	return promote_test();
}