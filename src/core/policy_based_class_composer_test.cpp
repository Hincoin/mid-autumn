#include "simple_test_framework.hpp"

bool policy_composer_test();


REGISTER_TEST_FUNC(policy_composer,policy_composer_test)

#include "PolicyBasedClass.hpp"

using namespace ma;
template<typename D,typename C>
class InterfaceTest:public C{
	D& derived(){return static_cast<D&>(*this);}
	const D& derived()const{return static_cast<const D&>(*this);}
	typedef InterfaceTest<D,C> class_type;
public:
	double test1(){
		//printf("has member function test1 %d\n", &class_type::test1 != &C::test1);
		return derived().test1Impl();
	}

	typename C::test_t test2()const {
		return derived().test2Impl();
	}
};


template<typename C>
class Implementation1:public C /*:public InterfaceTest<Implementation<C>,C>*/
{
	char a[10];
public:
	double test1(){ //this will override the test1Impl
		return 1.5;
	}
	double test1Impl(){
		return 2.1;
	};
};


template<typename C>
struct  Implementation2:public C /*:public InterfaceTest<Implementation<C>,C>*/
{

public:
	//void test1(){}
	typename C::test_t test2Impl()const{
		int a=0;
		return a;
	}
};

struct Conf{
	typedef int test_t;
};
template<typename BaseP >
struct policya:public BaseP{
	typename BaseP::test_t test2Impl()const{
		return 3;
	}
};
template<typename B>
struct policyb:public B{
	double test1Impl(){
		return 2.5;
	}
};

template<typename D,typename C>
struct interfaceTest2:public C{
	typedef typename C::test_t test_t;
	test_t test()const{
		return test1() + test2();
	}
protected:
	D& derived(){return static_cast<D&>(*this);}
	const D& derived()const{return static_cast<const D&>(*this);}
	test_t test1()const{return derived().test1Impl();}
	test_t test2()const{return derived().test2Impl();}
};

template<typename C>
struct testpolicy2a:public C{
	typename C::test_t
		test1Impl()const{return 10;}
};
template<typename C>
struct testpolicy2b:public C{
	typename C::test_t
		test2Impl()const{return 13;}
};

bool policy_composer_test(){
	typedef class_composer<Conf,InterfaceTest,Implementation1,Implementation2> my_type; 
	typedef class_composer<Conf,InterfaceTest,policya,policyb> my_type2_t;
	typedef class_composer<Conf,interfaceTest2,testpolicy2a,testpolicy2b> my_type3_t;
	bool result = true;
	my_type t;
	result = t.test1() == 1.5;
	result = result && t.test2() == 0;

	my_type2_t t1;
	result = result && t1.test1() == 2.5;
	result = result && t1.test2() == 3;

	my_type3_t t2;
	result = result && t2.test() == 23;
	return result;
}