#include "pool.hpp"
#include "CRTPInterfaceMacro.hpp"

//#include "crtpinterfacemacro.hpp"
#include <cmath>
BEGIN_CRTP_INTERFACE(test_class)
CRTP_METHOD(int,retInt,0,())

CRTP_METHOD(double,addDouble,2,(I_(double,a0),I_(double,a1)))

CRTP_CONST_METHOD(double,multi,2,(I_(double,a0),I_(double,a1)))

CRTP_CONST_VOID_METHOD(print5sum,5,
					   (I_(float , a0),
					   I_(float,a1),
					   I_(float,a2),
					   I_(float,a3),
					   I_(float,a4)))

CRTP_TEMPLATE_METHOD(1,double,template_1,2,(I_(T0,a),I_(T0&,b)))

CRTP_TEMPLATE_METHOD(2,float,template_2,2,(I_(T0,a),I_(T1,b)))

CRTP_CONST_TEMPLATE_METHOD(3,float,template_3,2,(I_(T0,a),I_(T1,b)) )

CRTP_CONST_VOID_TEMPLATE_METHOD(4,template_4,3,(I_(T0,a),I_(T1,b),I_(T2,c)) )

END_CRTP_INTERFACE

#include <stdio.h>
//#include 
#include <stdlib.h>
#include <typeinfo>
template<typename CFG>
class Impl:public test_class<Impl<CFG>,CFG>
{
	friend class test_class<Impl<CFG>,CFG>;

	int retIntImpl(){return 100;}
	double addDoubleImpl(double a0,double a1){return a0+a1;}
	double multiImpl(double a0,double a1)const{return a0*a1;}
	void print5sumImpl(float a0,float a1,float a2,float a3,float a4)const{
		printf("print5sum:%f+%f+%f+%f+%f = %f \n",a0,a1,a2,a3,a4,a0+a1+a2+a3+a4);
	}
	template<typename AT>
	double template_1Impl(AT a,AT& b){	printf("template_1 : %f \n",double(a+b));return 0;}
	template<typename AT0,typename ATT>
	float template_2Impl(AT0 a,ATT b){ printf("template_2 %s,%s\n",typeid(AT0).name(),typeid(ATT).name());return 0;}
	template<typename A0,typename AA,typename AC>
	float template_3Impl(A0 a, AA b)const
	{printf("template_3:%s,%s,%s\n",typeid(A0).name(),typeid(AA).name(),typeid(AC).name());return 0;}
	template<typename A0,typename A1,typename A2,typename A3>
	void template_4Impl(A0 a,A1 b,A2 c)const{
		printf("template_4: %s,%s,%s,%s \n",typeid(A0).name(),typeid(A1).name(),typeid(A2).name(),typeid(A3).name());
	}
};
#include <boost/preprocessor/stringize.hpp>
bool crtp_test()
{
	Impl<void> some_case;
	typedef test_class<Impl<void>,void> parent_class;
	printf("-------------------------------\n");
	float a0=rand()/2.f;
	float a1=rand()/2.f;
	float a2=rand()/2.f;
	float a3=rand()/2.f;
	float a4=rand()/2.f;
	printf("retInt:%d \n",some_case.retInt());
	printf("addDouble:%f+%f=%f;\n",a0,a1,some_case.addDouble(a0,a1));
	printf("multi:%f*%f=%f;\n",a3,a4,some_case.multi(a3,a4));
	some_case.print5sum(a0,a1,a2,a3,a4);


	some_case.template_1(a0,a1);
	some_case.template_2(1,2.3f);
	some_case.template_3<char,int,float>('a',1);
	some_case.template_4<char,short,int,double>('a',255,0);
	printf("-------------------------------\n");
	char* x = BOOST_PP_STRINGIZE((CRTP_CONST_METHOD(spectrum_t,sample_l,7,(
		 I_(const scene_ptr,s),
		 I_(scalar_t,u1),
		 I_(scalar_t,u2),
		 I_(scalar_t,u3),
		 I_(scalar_t,u4),
		 I_(ray_t&,r),
		 I_(scalar_t&,pdf)
		))));
	return true;
}
#include "simple_test_framework.hpp"

REGISTER_TEST_FUNC(crtp,crtp_test)