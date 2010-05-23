#include "boost/mpl/for_each.hpp"
#include "net_ext.hpp"
#include "NetImageFilm.hpp"


namespace ma{


	void register_all_net_ext()
	{
	
	#define REGISTER_ALL_TYPES(Types,F)\
		boost::mpl::for_each<boost::mpl::transform<Types,identity_wrapper<boost::mpl::_1> >::type>(F());\

		typedef boost::mpl::vector<NetImageFilm<> > net_film_types;
		REGISTER_ALL_TYPES(net_film_types,register_film_func)
	}



}
