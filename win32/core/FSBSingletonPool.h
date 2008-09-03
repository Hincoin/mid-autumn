#ifndef FSBSINGLETONPOOL_H
#define FSBSINGLETONPOOL_H

#include "FSBAllocator.h"

#include <boost/pool/singleton_pool.hpp>

#include "Mutex.h"

namespace ma
{
namespace core
{
	template<typename Tag, unsigned RequestedSize,
		typename UserAllocator = FSBAllocator<char[RequestedSize]>,
		typename Mutex = boost::details::pool::default_mutex>
	struct FSBSingletonPool{

		typedef typename UserAllocator::size_type size_type;

	private:
		struct SelfPool:Mutex,UserAllocator{
		};
		typedef boost::details::pool::singleton_default<SelfPool> Singleton;

		template<typename T>struct Tag{};
		static bool release_memory(Tag<FSBAllocator<char[RequestedSize]> >&)
		{
			return false;
		}
		static bool release_memory(Tag<FSBAllocator2<char[RequestedSize]> >&)
		{
			SelfPool & p = Singleton::instance();
			boost::details::pool::guard<Mutex> g(p);
			p.cleanSweep();
			return true;
		}

		
	public:
		static void * malloc()
		{
			SelfPool & p = Singleton::instance();
			boost::details::pool::guard<Mutex> g(p);
			return p.allocate(1);
		}

		static void * ordered_malloc(const size_type n)
		{
			SelfPool & p = Singleton::instance();
			boost::details::pool::guard<Mutex> g(p);
			return p.allocate(n);
		}
		static void free(void * const ptr)
		{
			SelfPool & p = Singleton::instance();
			boost::details::pool::guard<Mutex> g(p);
			p.deallocate(ptr);
		}
		static bool release_memory()
		{
			return release_memory(Tag<UserAllocator>());
		}
		
	};
}
}

#endif