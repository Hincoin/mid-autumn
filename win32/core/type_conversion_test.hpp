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
	typedef make_promotion<short>::type int_promote;
	std::cout<<typeid(int_promote).name();
};