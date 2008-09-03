// $Id:   $

#ifndef COREOBJECT_H
#define COREOBJECT_H


namespace ma
{
namespace core
{
	template<class MemoryManager>
	class DefaultMemPolicy{
	public:
		static void * operator new(size_t size);
		static void operator delete(void *rawmemory, size_t size);
		static void *operator new[]( size_t n )
		{ return ::operator new[](n); }
		static operator delete[]( void *p, size_t )
		{ ::operator delete[](p); }
	protected:
		DefaultMemPolicy(){}
		~DefaultMemPolicy(){}
	};

	class NullType{
	private:
		NullType(){}
		~NullType(){}
		NullType(const NullType&);
		NullType& operator=(const NullType&){}
	};

	template<typename Derived=NullType,class MemoryPolicy>
	class CoreObject:public MemoryPolicy
	{
		
	};
}
}
#endif