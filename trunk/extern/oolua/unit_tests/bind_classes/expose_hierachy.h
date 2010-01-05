#ifndef EXPOSE_HEIRACHY_H_
#	define EXPOSE_HEIRACHY_H_

#include "oolua.h"
#include "cpp_hierachy.h"

LUA_PROXY_CLASS(Abstract1)
	OOLUA_TYPEDEFS Abstract OOLUA_END_TYPES
	LUA_MEM_FUNC(void(),func1)
	LUA_MEM_FUNC(void(int,int,int),virtualVoidParam3Int)
LUA_PROXY_CLASS_END


LUA_PROXY_CLASS(Abstract2)
	OOLUA_TYPEDEFS Abstract OOLUA_END_TYPES
	LUA_MEM_FUNC(void(),func2_1)
LUA_PROXY_CLASS_END


LUA_PROXY_CLASS(Abstract3)
	OOLUA_TYPEDEFS Abstract OOLUA_END_TYPES
	LUA_MEM_FUNC(void(),func3_1)
LUA_PROXY_CLASS_END

BOOST_STATIC_ASSERT( (mpl::empty<OOLUA::Proxy_class<Abstract3>::Bases>::value));

LUA_PROXY_CLASS(Derived1Abstract1,Abstract1)
	OOLUA_NO_TYPEDEFS
LUA_PROXY_CLASS_END

LUA_PROXY_CLASS(TwoAbstractBases, Abstract1, Abstract2)
	OOLUA_NO_TYPEDEFS
LUA_PROXY_CLASS_END


LUA_PROXY_CLASS(DerivedFromTwoAbstractBasesAndAbstract3,TwoAbstractBases,Abstract3)
	OOLUA_NO_TYPEDEFS
LUA_PROXY_CLASS_END

namespace BASE_HELPERS
{
	//NOTE: Base class order is defined and used in OOLUA code.
	//Immediate bases come first in the order defined in the OOLUA_BASES_START to OOLUA_BASES_END block
	//then the bases of these base classes again in the order they were defined.
	inline std::vector<std::string>allBasesOfDerivedFromTwoAbstractBasesAndAbstract3()
	{
		std::vector<std::string> v;
		v.push_back("TwoAbstractBases");
		v.push_back("Abstract3");
		v.push_back("Abstract1");
		v.push_back("Abstract2");
		return v;
	}

	//Order for roots is defined (due to the nature in which the list is generated in OOLUA code), yet never used.
	//A root base is a base of class which has no bases itself.
	//If a class which is registered has no bases then it is not a root to itself yet can be a root for other classes.
	inline std::vector<std::string> rootBasesOfDerivedFromTwoAbstractBasesAndAbstract3()
	{
		std::vector<std::string> v;
		v.push_back("Abstract3");
		v.push_back("Abstract1");
		v.push_back("Abstract2");
		return v;
	}
}
#endif
