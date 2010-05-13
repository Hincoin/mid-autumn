#include "simple_test_framework.hpp"

bool ptr_var_test();


REGISTER_TEST_FUNC(ptr_var,ptr_var_test)

bool func_test();
bool ptr_var_test(){
	return func_test();
}


#include <boost/variant.hpp>
#include <boost/call_traits.hpp>
#include <boost/preprocessor.hpp>


#include <iostream>
#include <ctime>


#include <vector>
#include <algorithm>
#include "ptr_var.hpp"
#include <set>
#include <boost/pool/pool_alloc.hpp>
int cost_fun(int n)
{
	if (n<1) return 1;
	return cost_fun(n-1) + cost_fun(n-2);
}
namespace ma{

	MAKE_VISITOR(t0,0)
		MAKE_VISITOR(matchesFlags,1)
		MAKE_VISITOR(t0,1)
		MAKE_VISITOR(t,1)
		MAKE_VISITOR(tt,2)
		MAKE_VISITOR(ttt,1)

		namespace decl_func{
		DECL_FUNC(void,t0,0)
		DECL_FUNC(int,t,1);
		DECL_FUNC(float,tt,2)
}
	struct test_obj;
	struct v_test_obj{
		virtual void t0()const=0;
		virtual int t(int n)const=0;
		virtual float tt(float& d1,float d2)=0;
		virtual void ttt(const test_obj& o)=0;
		virtual ~v_test_obj(){}
	}; 

	struct test_obj
	{
		int a[1];
	public:
		//test_obj(){}
		bool matchesFlags(int flags)const{ return true;}
		void t0()const{ cost_fun(10);}
		int t(int n)const{return cost_fun(n);}
		float tt(float& d1,float d2){
			d1+=d2;
			return d1;
		}
		void ttt(const test_obj& o){
			o.t0();
		}
		~test_obj(){}
	protected:
	private:
	};
	struct test_obj1:test_obj{};
	struct test_obj2:test_obj{};
	struct test_obj3:test_obj{};
	struct test_obj4:test_obj{};
	struct test_obj5:test_obj{};
	struct test_obj6:test_obj{};
	struct test_obj7:test_obj{};
	struct test_obj8:test_obj{};

	struct v_test_obj1:v_test_obj{
		int a[1];
	public:
		bool matchesFlags(int flags)const{ return true;}
		virtual void t0()const{ cost_fun(10);}
		virtual int t(int n)const{return cost_fun(n);}
		virtual float tt(float& d1,float d2){
			d1+=d2;
			return d1;
		}
		virtual void ttt(const test_obj& o){
			o.t0();
		}
		virtual ~v_test_obj1(){}
	};
}

using namespace ma;

struct vi{
	typedef int result_type;
	template<typename T>
	result_type operator()(T* x)
	{
		return *x;
	}
};
using namespace std;
inline size_t t0_test_impl(const vector<v_test_obj*>& v)
{
	for (size_t i = 0;i < v.size();++i)v[i]->t0();
	for (size_t i = 0;i < v.size();++i)delete v[i];
	return v.size();
}
size_t t0_test(int n)
{
	vector<v_test_obj*> v;
	while( n-- > 0)v.push_back(new v_test_obj1);
	return t0_test_impl(v);

}
template<typename P>
size_t t0_test(int n)
{
	vector<P> v;
	while (n-- > 0)
	{
		v.push_back(new test_obj1);
	}
	//std::for_each(v.begin(),v.end(),apply_visitor<P>(make_t0_visitor_ref<void>()));
	//t0_ref<void>(*v.begin());
	for (size_t i = 0;i < v.size();++i)t0_ref<void>(v[i]);
	//for (int i = 0;i < v.size();++i)delete_ptr(v[i]);
	//free up
	//std::for_each(v.begin(),v.end(),apply_visitor<P>(ptr_deleter<P>()));
	for (typename vector<P>::iterator it = v.begin();it != v.end(); ++it)
	{
		//delete get<test_obj1*>(*it);
		delete_ptr(*it);
	}
	return v.size();
}
bool func_test()
{
	bool result = true;
	typedef boost::mpl::list<test_obj2,test_obj3,test_obj4,test_obj5,test_obj1,test_obj6,test_obj7,test_obj8,test_obj> var_seq;
	typedef ma::make_ptr_var_over_sequence<var_seq>::type var_t;

	BOOST_STATIC_ASSERT((is_ptr_variant<var_t>::value));
	var_t d;
	d = new test_obj;
	test_obj *test_ptr = new test_obj;
	t0_ref<void>(d);
	result = t_ref<int>(d,2) == t_ref<int>(*test_ptr,2);
	result = result && t_ref<int>(d,2) == test_ptr->t(2);
	std::cout<<t_ref<int>(d,2)<<std::endl;
	float a = 1,b = 3;
	float c=a,c1=b;
	result = result && tt_ref<float>(d,a,b) == tt_ref<float>(*test_ptr,c,c1);;
	result = result && tt_ref<float>(d,a,b) == test_ptr->tt(c,c1);

	///////////////////////////////////
	decl_func::t0(d);
	decl_func::tt(d,a,b);
		//////////////////////////////
	test_obj o_test;
	
	get<test_obj>(o_test);
	get<test_obj&>(o_test);
	get<const test_obj>(o_test);
	get<const test_obj&>(o_test);

	ttt_ref<void>(d,get<const test_obj&>(d));
	ttt_ref<void>(d,get<test_obj&>(d));
	ttt_ref<void>(d,get<test_obj>(d));
	ttt_ref<void>(d,get<test_obj&>(d));
	ttt_ref<void>(d,*get<const test_obj*>(d));
	ttt_ref<void>(d,*get<test_obj*>(d));
	ttt_ref<void>(o_test,get<const test_obj&>(d));
	ttt_ref<void>(test_ptr,get<const test_obj&>(d));
	//std::cout<<t<int(int)>(d,2)<<std::endl;
	result = result && t<int(int)>(d,2) == t<int(int)>(test_ptr,2);
	result = result && t<int(int)>(d,2) == test_ptr->t(2) && matchesFlags_ref<bool>(d,1);

	ttt<void(const test_obj&)>(d,get<const test_obj&>(d));
	ttt<void(const test_obj&)>(d,get<test_obj&>(d));
	ttt<void(const test_obj&)>(d,get<test_obj>(d));
	ttt<void(const test_obj&)>(d,get<test_obj&>(d));
	ttt<void(const test_obj&)>(d,*get<const test_obj*>(d));
	ttt<void(const test_obj&)>(d,*get<test_obj*>(d));
	ttt<void(const test_obj&)>(o_test,get<const test_obj&>(d));
	ttt<void(const test_obj&)>(test_ptr,get<const test_obj&>(d));
 
	std::set<var_t> v_set;
#ifdef NDEBUG
#define ENABLE_PERFORMANCE_TEST
#endif
#ifdef	ENABLE_PERFORMANCE_TEST
	const int N = 1024*256 * 128;
	const int M = 1;
#else
	const int N = 128;
	const int M = 1;
#endif
	int i = 0;
	clock_t s = clock();
	while(i++ < N)
		t0_test<test_obj*>(M);
	printf("t0_test<test_obj*>() : %ld \n",(long)(clock()-s));
	i=0;
	s =clock();
	while (i++ < N)
	{
		t0_test(M);
	}
	printf("t0_test<v_test_obj*>() : %ld \n",(long)(clock()-s));
	i = 0;
	s = clock();
	while (i ++ < N)
	{
		t0_test<var_t>(M);
	}
	printf("t0_test<var_t>() : %ld",(long)(clock()-s));
	typedef ma::make_shared_ptr_var_over_sequence<var_seq>::type shared_ptr_var_t;
	BOOST_STATIC_ASSERT(is_ptr_variant<shared_ptr_var_t>::value);
	BOOST_MPL_ASSERT((boost::is_same<test_obj2,boost::mpl::at_c<shared_ptr_var_t::types,0>::type>));
	var_t tt = new test_obj;
	shared_ptr_var_t shared_tt(tt);
	
	return result;
}



//////////////////////////////////////////////////////////////////////////

struct add{};
struct sub{};
template <typename OpTag> struct binary_op;

typedef ptr_var<
int
, binary_op<add>
, binary_op<sub>
> expression;

template <typename OpTag>
struct binary_op
{
	expression left;  // variant instantiated here...
	expression right;

	binary_op( const expression & lhs, const expression & rhs )
		: left(lhs), right(rhs)
	{
	}

};
binary_op<add> a(expression(new int(1)),expression(new int(2)));


//////////////////////////////////////////////////////////////////////////
template<typename Conf>
struct primitive{
	typedef typename Conf::primitive_ptr primitive_ptr;

	std::vector<primitive_ptr> ps;
};

struct primitive_config 
{
	typedef primitive_config class_type;
	typedef ptr_var<primitive<class_type> > primitive_ptr;
};

primitive<primitive_config> recurse_type;
struct incomplete;
typedef ptr_var<primitive<incomplete> > incomplete_ptr;incomplete_ptr iptr;
//struct incomplete :primitive_config
//{
//};

//////////////////////////////////////////////////////////////////////////
