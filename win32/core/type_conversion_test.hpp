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

template<typename T>
void print_type()
{
std::cout<<typeid(T).name()<<std::endl<<sizeof(T)<<std::endl;
}
void promote_test(){
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
};