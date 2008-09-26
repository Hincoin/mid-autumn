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
	class PooledCoreObject:public MemoryPolicyType::template MemoryPolicy<PooledCoreObject<Derived> >
	{
	public:
		typedef Derived DerivedType;

		char c;
	};


}
}
#endif