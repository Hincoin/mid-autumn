#ifndef MA_TYPECONVERSION_HPP
#define MA_TYPECONVERSION_HPP


#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/variant.hpp>

#include <boost/static_assert.hpp>

#include <boost/mpl/max_element.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/sizeof.hpp>

#include <boost/mpl/set.hpp>
#include <boost/mpl/copy.hpp>
#include <boost/mpl/insert.hpp>

//meta-function for type promotion

//no dead-loop promotion allowed
//such as a promote to b , b promote to c, c promote to a

//multiple promotion need to be done


template<typename T>
struct type_promote{
	//typedef boost::mpl::vector<T> type;
	typedef T type;
};

template<typename T>
struct promote_types{
private:
	struct details{
		template<typename U>
		struct is_top_promote:boost::is_same<U,typename type_promote<U>::type>
		{};

		template<typename U,bool>
		struct do_promote;

		template<typename U>
		struct do_promote<U,true >{
			typedef boost::mpl::set<> type;
		};

		template<typename U>
		struct do_promote<U,false>
		{
			//dead loop check to see if U is already in the mpl::vector
			
			typedef typename boost::mpl::insert<
				typename do_promote<
				typename type_promote<U>::type ,is_top_promote<U>::value
				>::type,U
			>::type type;
		};
	};
	typedef typename details::do_promote<T,details::is_top_promote<T>::value>::type types;
public:
	typedef types type;
};
template<typename T>
struct promote_to_variant{
public:
	typedef typename boost::make_variant_over< typename promote_types<T>::type >::type type;
};

template<typename Sequence>
struct biggest_type
{
private:
	typedef boost::mpl::_1 _1;
	typedef typename boost::mpl::max_element<
		boost::mpl::transform_view<Sequence,boost::mpl::sizeof_<_1> >
	>::type result_iter;
public:
	typedef typename boost::mpl::deref<typename result_iter::base>::type type;
};




#endif
