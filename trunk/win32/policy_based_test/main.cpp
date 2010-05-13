#include "PolicyBasedClass.hpp"
#include <stdio.h>

#include "compiler_optimization.h"
using namespace ma;


struct AT{
	void test1Impl(){}
};
struct BT:AT{
	void test1Impl(){}
};

template<typename D,typename C>
class InterfaceTest:public C{
	D& derived(){return static_cast<D&>(*this);}
	const D& derived()const{return static_cast<const D&>(*this);}
	typedef InterfaceTest<D,C> class_type;
public:
	void test1(){
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
	void test1(){
		printf("override test1\n");
	}
void test1Impl(){
printf("test1\n");
};
};

 
template<typename C>
struct  Implementation2:public C /*:public InterfaceTest<Implementation<C>,C>*/
{
	
public:
	//void test1(){}
	typename C::test_t test2Impl()const{
		printf("test2\n");int a=0;
		return a;
	}
};

struct Conf{
	typedef int test_t;
};
template<typename BaseP >
struct policya:public BaseP{
};
template<typename B>
struct policyb:public B{
	void test1Impl(){
		printf("Base : %s \n",typeid(B).name());
		return 0;
	}
};
#include <typeinfo>
int main()
{

	//typedef config_template_classes<Conf,policya,policyb>::type my_type;
	//my_type t;
	//t.a=1;

	
	//t.testImpl();
//typedef config_template_classes<Conf,Implementation>::type my_types;
	typedef class_composer<Conf,InterfaceTest,Implementation1,Implementation2> my_type; 
	my_type t;
	t.test1();
	t.test1Impl();
	t.test2();
	//BOOST_STATIC_ASSERT( (HAS_MEMBER_FUNCTION(my_type,void,test1Impl,()) ) );
	printf("             %d\n",boost::is_empty<InterfaceTest<my_type,Conf> >::value);
	printf("is empty: %d, %s\n",boost::is_empty< my_type >::value,typeid(my_type).name());
//my_type a;
//a.test();
//printf("%d %d %d %d\n ",sizeof(my_type),sizeof(generate_hiearachy<boost::mpl::list<A> >),sizeof(K<A,B/*,C*/>),compiler_optimisation_traits::supportsEBO7);
//	typedef details::policy_based_class::apply_stub<Implementation>::type k_type;
//	printf("is empty: %d\n",boost::is_empty< N >::value);
BOOST_STATIC_ASSERT((boost::is_same<details::policy_based_class::void_<int>,details::policy_based_class::void_<int> >::value));
}
