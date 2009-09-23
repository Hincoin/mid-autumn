#ifndef _INCLUDED_SINGLETON_POOL_HPP_
#define _INCLUDED_SINGLETON_POOL_HPP_
#include "Singleton.hpp"
#include "memory_pool.hpp"
#include "details/pool_mutex.hpp"
namespace ma{
	namespace core{

		//
		// The singleton_pool class allows other pool interfaces for types of the same
		//   size to share the same pool
		//
		template <typename Tag, unsigned RequestedSize,
			typename Mutex=details::null_mutex,
			unsigned NextSize=32,
			typename Pool = fixed_pool
		>
		struct fixed_singleton_pool
		{
		public:
			typedef Tag tag;
			typedef Mutex mutex;
			typedef typename Pool::size_type size_type;
			typedef typename Pool::difference_type difference_type;

			BOOST_STATIC_CONSTANT(unsigned, requested_size = RequestedSize);
			BOOST_STATIC_CONSTANT(unsigned, next_size = NextSize);

		private:
			struct pool_type: Mutex
			{
				Pool p;
				pool_type():p(RequestedSize, NextSize) { }
			};

			typedef simple_singleton<pool_type> singleton;

			fixed_singleton_pool();

		public:
			static void * alloc()
			{
				pool_type & p = singleton::instance();
				details::scope_lock<Mutex> g(p);
				return p.p.alloc();
			}
			static void free(void * const ptr)
			{
				pool_type & p = singleton::instance();
				details::scope_lock<Mutex> g(p);
				p.p.free(ptr);
			}
			static bool release_memory()
			{
				pool_type & p = singleton::instance();
				details::scope_lock<Mutex> g(p);
				return p.p.release_memory();
			}
		};
		template <typename Tag,
			typename Mutex,
			typename Pool = memory_pool<Mutex>
		>
		struct generic_singleton_pool
		{
		public:
			typedef Tag tag;
			typedef Mutex mutex;
			typedef typename Pool::size_type size_type;
			typedef typename Pool::difference_type difference_type;
		private:
			struct pool_type//: Mutex
			{
				Pool p;
			};

			typedef simple_singleton<pool_type> singleton;

			generic_singleton_pool();

		public:
			static void * alloc(size_type size)
			{
				pool_type & p = singleton::instance();
				//details::scope_lock<Mutex> g(p);
				return p.p.alloc(size);
			}
			static void free(void * const ptr)
			{
				pool_type & p = singleton::instance();
				//details::scope_lock<Mutex> g(p);
				p.p.free(ptr);
			}
			static void free(void * const ptr,size_type sz)
			{
				pool_type & p = singleton::instance();
				//details::scope_lock<Mutex> g(p);
				p.p.free(ptr,sz);
			}
			static bool release_memory()
			{
				pool_type & p = singleton::instance();
				//details::scope_lock<Mutex> g(p);
				return p.p.release_memory();
			}
		};
	}
}
#endif