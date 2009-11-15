//
//	product_view.h
//

//#ifndef	_product_view_
//#define	_product_view_

//#endif
//
#include "CartesianProductView.hpp"	
#include <iostream>
#include <boost/mpl/list.hpp>
#include <boost/mpl/list_c.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/size.hpp>

#include "simple_test_framework.hpp"
#include <typeinfo>
using namespace boost::mpl;
//using boost::mpl::list;
//using  boost::mpl::for_each;
//using boost::mpl::list_c;
//using boost::mpl::push_front;
////using boost::mpl::list_c;
////using boost::mpl::list_c;
////using boost::mpl::list_c;
typedef	list< 
list_c< int, 1, 16,17,18>,
list_c< int, 2, 3 >, 
list_c< int, 4, 5, 6 > ,
list_c< int, 7, 8, 9, 10 > ,
list_c< int, 11,12,13,14,15>/*,
list_c< int, 16,17,18,19,20,21>  ,
list_c< int, 22,23,24,25,26,27,28> ,
list_c< int, 29,30,31,32,33,34,35,36> ,
list_c< int, 37,38,39,40,41,42,43,44,45,46> ,*/
>	Six;

typedef push_front< Six, list<> >::type	None;

void print1( int c )
{
	std::cout << c << ' ';
}

struct print
{
	template<typename L> void operator()( L ) {
		std::cout << "[ ";
		for_each<L>( print1 );
		std::cout << "]" << std::endl;
	}
};


bool product_view_test();

REGISTER_TEST_FUNC(product_view_test,product_view_test)
bool product_view_test()
{
	//std::cout << "Source lists (1)\n";
	//for_each<Six>( print() );
	//std::cout << "Permutations\n";
	//for_each< product_view<Six> >( print() );

	//std::cout << "Source lists (2)\n";
	//for_each<None>( print() );
	//std::cout << "Permutations\n";
	//for_each< product_view<None> >( print() );
	//typedef push_front<Six,Six>::type SixbySix;
	//typedef push_front<Six,SixbySix>::type SixbySixbySix;
	//for_each<product_view<Six> >(print() );
	//BOOST_MPL_ASSERT((boost::mpl::is_sequence(recured_viewed_combination_t)));
	return true;
}
