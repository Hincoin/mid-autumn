#ifndef INCLUDE_ADD_POINTER_HPP
#define INCLUDE_ADD_POINTER_HPP

#include <boost/type_traits/add_pointer.hpp>
namespace ma{
	template<typename T>
	struct add_ptr:boost::add_pointer<T>{};

	//if you want to use shared_ptr for some class specialize with:
	//template<>
	//struct add_ptr<T>{
	//	typedef shared_ptr<T> type;
	//};
}
#endif