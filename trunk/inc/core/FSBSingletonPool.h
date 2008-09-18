#ifndef FSBSINGLETONPOOL_H
#define FSBSINGLETONPOOL_H

#include "FSBAllocator.h"

#include <boost/pool/singleton_pool.hpp>

#include "Mutex.h"

namespace ma
{
namespace core
{
	template<int N>
	struct SIZE_N{ char a[N];};

	//this pool is pretty good at RequestedSize is <= 512 and not capable of continuous storage allocation
	template<typename SingletonTag,unsigned int RequestedSize,
		typename UserAllocator = FSBAllocator<char[RequestedSize] >,
		typename Mutex = boost::details::pool::default_mutex>
	struct FSBSingletonPool{

		typedef typename UserAllocator::size_type size_type;
		typedef typename UserAllocator::value_type value_type;
	private:
		struct SelfPool:Mutex,UserAllocator{
		};
		typedef boost::details::pool::singleton_default<SelfPool> Singleton;

	  template<typename T>struct Tag{typedef T type;};
		static bool release_memory(Tag<FSBAllocator<value_type> >)
		{
			return false;
		}
		static bool release_memory(Tag<FSBAllocator2<value_type> >)
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
		static void free(void * const ptr)
		{
			SelfPool & p = Singleton::instance();
			boost::details::pool::guard<Mutex> g(p);
			p.deallocate(reinterpret_cast<value_type*>(ptr),1);
		}
		static bool release_memory()
		{
			return release_memory(Tag<UserAllocator>());
		}
		
	};

}
}

#endif
