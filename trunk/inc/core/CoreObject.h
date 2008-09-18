// $Id:   $

#ifndef COREOBJECT_H
#define COREOBJECT_H

#include "NullType.h"
#include "MemoryPolicy.h"


namespace ma
{
namespace core
{
	template<typename Derived = EmptyType>
	class CoreObject:public MemoryPolicyType::template MemoryPolicy<CoreObject<Derived> >
	{
	public:
		typedef Derived DerivedType;

		char c;
	};


}
}
#endif