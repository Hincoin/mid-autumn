#ifndef MAMEMORYPOOL_H
#define MAMEMORYPOOL_H

#include <boost/pool/singleton_pool.hpp>

//designed by lzy for big object allocation, tested under 32bit arch
namespace ma
{
namespace core
{
	//template<typename ContainerAllocator>
	struct MAMemoryPool{
		static void* malloc(std::size_t sz);
		static void free(void* p);//mark as freed
		static bool clean_unused(); // this is very slow
		static void defragment();//defragment 
	};
	template<typename Mutex>
	struct MAMemPoolSingleton{
		struct SelfPool:Mutex,MAMemoryPool{};
		typedef boost::details::pool::singleton_default<SelfPool> Singleton;

		typedef std::size_t size_type;
	public:
		static void * malloc(size_type s)
		{
			SelfPool & p = Singleton::instance();
			boost::details::pool::guard<Mutex> g(p);
			return p.malloc(s);
		}
		static void free(void * const ptr)
		{
			SelfPool & p = Singleton::instance();
			boost::details::pool::guard<Mutex> g(p);
			p.free(ptr);
		}
		static bool release_memory()
		{
			SelfPool & p = Singleton::instance();
			boost::details::pool::guard<Mutex> g(p);
			p.clean_unused();
		}

	};
}
}
#endif