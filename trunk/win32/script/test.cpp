#include <iostream>

#include <boost/bimap.hpp>

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

const void* test(void* p);

int main(){

	D d;
	d.a = 10;
	d.b = 1.9;
	Base1* b = static_cast<Base1*>((void*)((Base1*)&d));
	std::cout<<b->a<<std::endl;

	Base1& bb = static_cast<Base1&>(d);

	void* bb_p = &bb;
	bb = *static_cast<Base1*>(bb_p);

	std::cout<<bb.a<<std::endl;

	const char& c = static_cast<const char&>(d.b);
	std::cout<<c<<std::endl;

	const void* c_p = &static_cast<const char&>(d.b);

	const void* (*test_ptr)(void*) = &test;
	const void* c_pp = (*test_ptr)(&d.b);
	const char& cc = *static_cast<const char*>(c_pp);

	std::cout<<(int)cc<<std::endl;
}

const void* test(void* p)
{
	int a[100];
	const void* c_p = &static_cast<const char&>(*((double*)p));

	typedef boost::bimap<int,char> ic_map;
	ic_map a1212 ;
	return c_p;
}