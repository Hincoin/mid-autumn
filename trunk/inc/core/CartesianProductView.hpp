#ifndef _MA_INCLUDED_CARTESIANPRODUCTVIEW_HPP_
#define _MA_INCLUDED_CARTESIANPRODUCTVIEW_HPP_

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
					_1, //boost::is_same< begin<_2>, end<_2> > 
					 empty<_2> //refuses to compile on MSVC 7.1
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
#endif
