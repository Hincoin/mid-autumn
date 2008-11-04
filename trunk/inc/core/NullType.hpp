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

#ifndef NULLTYPE_HPP
#define NULLTYPE_HPP

#include <cassert>

		class NullType;

		class EmptyType{
		protected:
			EmptyType(){}
			~EmptyType(){}
			EmptyType(const EmptyType&){}
			EmptyType& operator=(const EmptyType&){}
		};
		template<int N>
		struct MultiEmptyType:EmptyType{
			enum{index = N};

			//template <typename T>
			//operator T()const{
			//	assert(false);
			//	return *((T*)0);
			//}
		};
#endif