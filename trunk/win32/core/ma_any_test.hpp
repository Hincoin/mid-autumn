#ifndef MA_ANY_TEST_HPP
#define MA_ANY_TEST_HPP


#include "MAny.hpp"

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

struct Base3{
	int a ;
};
struct D1:Base3{
	double d;
};


template<>
struct multiple_type_promote<D>{
	typedef boost::mpl::vector<D,Base1,Base2> type;
};
template<>
struct multiple_type_promote<D1>
{
	typedef boost::mpl::vector<D1,Base3> type;
};

inline void many_test(){
	using namespace ma;
	Base3 b3;
	MAny a0(b3) ;
	a0.template cast<Base3>().a = 10;
	int c = a0.template cast<Base3>().a;

	//more test code here
}

#endif