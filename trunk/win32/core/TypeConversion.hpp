#ifndef MA_TYPECONVERSION_HPP
#define MA_TYPECONVERSION_HPP


#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/placeholders.hpp>

//meta-function for type promotion


template<typename T>
struct type_promote{
	typedef T type;
};

template<typename T>
struct make_promotion{
private:
	typedef typename type_promote<T>::type promoted_type;
	typedef typename type_promote<promoted_type>::type more_promoted_type;
	typedef typename boost::mpl::if_<
			boost::is_same<promoted_type,more_promoted_type >, //end
			boost::mpl::vector<promoted_type>,
			make_promotion<promoted_type>		
	> type_seq;
public:
	typedef typename boost::mpl::push_back<typename type_seq::type,T>::type type;

	template<typename U>
	struct apply{
		typedef U type;
	};
};



#endif
