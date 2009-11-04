//
//	product_view.h
//

//#ifndef	_product_view_
//#define	_product_view_

#include <boost/mpl/vector.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/copy.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/type_traits/is_same.hpp>

namespace boost{
	namespace mpl{
		namespace detail_ {

			using namespace boost::mpl;

			template<typename S, typename O> struct state_with_overflow
			{
				typedef	S	state;
				typedef O	overflow;
			};

			template<typename I3S> struct i3s_increment_inserter
			{
				template<typename I3, typename O> struct i3_
				{
					typedef	I3	i3;
					typedef	O	overflow;
				};

				template<typename I3> struct incr_
				{
					typedef	typename next< typename at_c< I3, 1 >::type >::type					next_;
					typedef	bool_< boost::is_same< next_, typename at_c<I3,2>::type >::value >	overflow_;

					typedef i3_<
						vector3< 
						typename at_c<I3,0>::type,
						typename if_< overflow_, typename at_c<I3,0>::type, next_ >::type,
						typename at_c<I3,2>::type
						>,
						overflow_
					>
					type;
				};

				typedef front_inserter< typename clear<I3S>::type >				base_;

				typedef	state_with_overflow< typename base_::state, true_ >		state;

				struct operation
				{
					template<typename S, typename I3> struct apply
					{
						typedef typename if_< 
							typename S::overflow, 
							typename incr_<I3>::type, 
							i3_<I3,false_> 
						>::type		next_;

						typedef state_with_overflow< 
							typename base_::operation::template apply< typename S::state, typename next_::i3 >::type, 
							typename next_::overflow 
						>	
						type;
					};
				};
			};

			template< typename SWO > struct product_iterator
			{
			private:

				typedef typename SWO::state	I3S;

			public:	
				typedef forward_iterator_tag category;

				typedef typename transform1< 
					I3S, deref< at< _1, int_<1> > >
				>::type type;

				typedef product_iterator<
					typename reverse_copy< I3S, i3s_increment_inserter<I3S> >::type
				>
				next;
			};

			template<typename S> struct product_view
			{
			private:

				typedef typename transform1<
					S, vector3< begin<_1>, begin<_1>, end<_1> >
				>::type
				begin_impl;

				typedef typename fold< 
					S, false_, 
					or_< 
					_1, boost::is_same< begin<_2>, end<_2> > 
					// empty<_2> refuses to compile on MSVC 7.1
					> 
				>::type
				empty_result;

			public:

				typedef product_view type;
				typedef nested_begin_end_tag tag;

				typedef product_iterator< state_with_overflow< begin_impl, empty_result > >	begin;
				typedef product_iterator< state_with_overflow< begin_impl, true_ > >		end;
			};

		}
		using detail_::product_view;
	}
}

//#endif
//
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
	for_each<product_view<Six> >(print() );
	//BOOST_MPL_ASSERT((boost::mpl::is_sequence(recured_viewed_combination_t)));
	return true;
}
