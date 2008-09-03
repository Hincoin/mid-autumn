// $Id:   $

#include "MemoryMgr.h"
#include "MemoryPool.h"

using namespace ma;
using namespace core;


#include <iostream>

class MemPolicy{
public:
	static void * operator new(size_t size){
		return ::operator new(size);
	}
	static void operator delete(void *rawmemory, size_t size)
	{
		
		return ::operator delete(rawmemory);
	}
	static void *operator new[]( size_t n)
	{
		std::cout<<"new[]:"<<n<<" " <<std::endl;
		return ::operator new[](n); 
	}
	static void operator delete[]( void *p, size_t s)
	{ 
		std::cout<<"delete[]:"<<s<<std::endl;
		::operator delete[](p); 
	}

public:
	MemPolicy(){}
	~MemPolicy(){}
};
class Derived:public MemPolicy
{
public:
	Derived(){};
	~Derived(){ std::cout<<"destroy derived"<<std::endl;}
};
int main()
{
	MemPolicy* p_mem = new MemPolicy;
	MemPolicy* a_mem = new MemPolicy[100];
	delete p_mem;
	delete[] a_mem;

	Derived* p_d = new Derived;
	delete p_d;
return 0;
}