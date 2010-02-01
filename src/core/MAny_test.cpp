//test MAny class

#include "simple_test_framework.hpp"

bool any_test();


REGISTER_TEST_FUNC(MAny,any_test)


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

struct Base3:D{
	int b3 ;
};
struct D1:Base3{
	double d;
};


template<>
struct multiple_type_promote<D>{
	typedef boost::mpl::vector<D,Base1,Base2> type;
};
template<>
struct multiple_type_promote<Base3>
{
	typedef boost::mpl::vector<D> type;
};
template<>
struct multiple_type_promote<D1>
{
	typedef boost::mpl::vector<D1,Base3> type;
};

inline bool many_test(){
	using namespace ma;
	Base3 b3;
	MAny a0(b3) ;
	a0.cast<Base3>().a = 10;
	int c = a0.cast<Base3>().a;

	bool result = true;
	result = c == 10;
	assert(result);
	//more test code here
	D1 d1;
	MAny a1(d1);
	a1.cast<Base1>().a = 10;
	int _10 = a1.cast<Base3>().a;
	result = result && (_10 == 10);
	assert(result);

	D d0;
	MAny a2(d0);

	Base1& base1 = a2.cast<Base1&>();
	Base2& base2 = a2.cast<Base2&>();
	D& d2 = a2.cast<D&>();
	base1.a = 10;
	base2.b = 11;
	d2.c = 10.5;
	result = result 
		&& (base1.a == d2.a && d2.a== 10)
		&& ( base2.b == d2.b );
	assert(result);

	D d_result = a2.cast<D>();
	d_result.a = 20;
	result = result 
		&& (d2.a == 10);
	a1 = a2;
	D d_result1 = a1.cast<D>();
	result = result && 
		(d_result1.a == 10);
	d_result1.a = 30;
	result = result && 
		(d_result.a == 20);
	d_result1 = a2.cast<D>();
	result = result && 
		(d_result.a == 20);
	assert(result);

	D1* void_ptr = new D1;
	MAny a_ptr(void_ptr);

	Base1* b1_ptr = a_ptr.cast<Base1*>();
	Base2* b2_ptr = a_ptr.cast<Base2*>();
	b1_ptr->a  = 10;
	b2_ptr->b = 11;
	D1* d_ptr = a_ptr.cast<D1*>();
	result = result &&
		(d_ptr->a == b1_ptr->a && b1_ptr->a == 10)
		&& 
		(d_ptr->b == b2_ptr->b && b2_ptr->b == 11);
	assert(result);
	MAny void_p ( (void*) void_ptr);
	d_ptr = void_p.cast<D1*>();
	result = result &&
		(d_ptr->a == b1_ptr->a && b1_ptr->a == 10)
		&& 
		(d_ptr->b == b2_ptr->b && b2_ptr->b == 11);
	assert(result);

	delete d_ptr;

	//basic type conversion
	MAny builtin(0.4);
	float x = builtin.cast<float&>();
	x = 3.5f;
	int cx = builtin.cast<int&>();
	cx = 5;
	char ccx = builtin.cast<char&>();
	ccx = 6;

	return result;
}

bool any_test(){
	return many_test();
}
