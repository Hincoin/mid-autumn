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

void promote_test(){
	typedef promote_to_variant<char>::type int_promote;
	typedef boost::variant<char,short,int,double> char_promote;
	std::cout<<typeid(char_promote).name()<<std::endl<<sizeof(char_promote)<<std::endl;

	typedef promote_types<char>::type char_promotions;
	typedef biggest_type<char_promotions>::type biggest_promotion;
	std::cout<<typeid(biggest_promotion).name()<<" "<<std::endl<<sizeof(biggest_promotion);
};