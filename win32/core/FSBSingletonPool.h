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
	template<typename Tag, unsigned RequestedSize,
		typename UserAllocator = FSBAllocator<char[RequestedSize] >,
		typename Mutex = boost::details::pool::default_mutex>
	struct FSBSingletonPool{

		typedef typename UserAllocator::size_type size_type;
		typedef typename UserAllocator::value_type value_type;
	private:
		struct SelfPool:Mutex,UserAllocator{
		};
		typedef boost::details::pool::singleton_default<SelfPool> Singleton;

		template<typename T>struct Tag{};
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

		static void * ordered_malloc(const size_type n)
		{
			assert(n==1);
			//SelfPool & p = Singleton::instance();
			//boost::details::pool::guard<Mutex> g(p);
			////for(size_t i =0;i < n;++i)
			//return p.allocate(1);
			//boost::singleton_pool<>
			return 0;
		}
		static void free(void * const ptr,size_t s)
		{
			SelfPool & p = Singleton::instance();
			boost::details::pool::guard<Mutex> g(p);
			p.deallocate(reinterpret_cast<value_type*>(ptr),s);
		}
		static bool release_memory()
		{
			return release_memory(Tag<UserAllocator>());
		}
		
	};

}
}

#endif