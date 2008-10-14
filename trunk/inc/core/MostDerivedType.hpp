////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008 by luozhiyuan (luozhiyuan@gmail.com)
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author makes no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

#ifndef MOSTDERIVEDTYPE_HPP
#define MOSTDERIVEDTYPE_HPP

#include <boost/mpl/if.hpp>
#include "NullType.hpp"

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