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
	D1 d1;
	MAny a1(d1);
	a1.template cast<Base3>().a = 10;
	int _10 = a1.template cast<Base3>().a;

	D d0;
	MAny a2(d0);

	Base1& base1 = a2.cast<Base1&>();
	Base2& base2 = a2.cast<Base2&>();
	D& d2 = a2.cast<D&>();
	base1.a = 10;
	base2.b = 11;
	d2.c = 10.5;

	D d_result = a2.cast<D>();

	a1 = a2;
	d_result = a1.cast<D>();
}

#endif