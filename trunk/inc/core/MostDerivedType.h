#ifndef MOSTDERIVEDTYPE_H
#define MOSTDERIVEDTYPE_H

#include <boost/mpl/if.hpp>
#include "NullType.h"

template<typename Derived>
struct MostDerivedType{
private:
	typedef typename Derived::DerivedType GrandDerived;
	//static const bool is_empty_ = boost::is_same<GrandDerived,EmptyType>::value;
public:
	typedef typename boost::mpl::if_<
		boost::is_same<GrandDerived,EmptyType>,
		Derived,
		typename MostDerivedType<GrandDerived>::type >::type type;
};
template<>
struct MostDerivedType<EmptyType>{
	typedef EmptyType type;
};
#endif