

#include "CRTPInterfaceMacro.hpp"


namespace ma{
	

	BEGIN_CRTP_INTERFACE(crtp_test)
		ADD_CRTP_INTERFACE_TYPEDEF(test_type)
		ADD_CRTP_INTERFACE_FUNC(test_type, test_func_interface_const,(int a,int b), (a,b), const)
		ADD_CRTP_INTERFACE_FUNC(void, test_func_interface,(int a,int b), (a,b), )
	END_CRTP_INTERFACE()


	
	template<typename Config>
	class crtp_test_impl:public crtp_test<crtp_test_impl<Config> , Config >
	{
	public:
		typedef typename Config::test_type test_type;

		test_type test_func_interface_const(int a ,int b)const{
			std::cout<<a<<","<<b<<" const"<<std::endl; return 0;
		}
		test_type test_func_interface(int a ,int b){std::cout<<a<<","<<b<<std::endl; return 0;}
	};

	template<typename T,typename InterfaceConfig>
	void test_crtp(const crtp_test<T,typename InterfaceConfig>& t)
	{
		typename crtp_test<T,typename InterfaceConfig>::test_type a = 0;
		typename crtp_test<T,typename InterfaceConfig>::test_type b = 1;

		t.test_func_interface_const(a,b);
		//t.test_func_interface(a,b);
	}



	struct crtp_test_impl_configure{
		typedef int test_type;
	};

	typedef crtp_test_impl<crtp_test_impl_configure> test_impl;

	inline void crtp_macro_test(){
		test_impl a ;
		test_crtp(a);
	}
}