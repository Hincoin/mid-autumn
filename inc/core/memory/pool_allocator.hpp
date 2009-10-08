#ifndef _INCLUDED_POOL_ALLOCATOR_HPP_
#define _INCLUDED_POOL_ALLOCATOR_HPP_

#include <limits>

#include "singleton_pool.hpp"

//std allocator based on memory pool
namespace ma{
	namespace core{
		struct pool_allocator_tag { };

		template <typename T,
			typename Mutex=details::null_mutex >
		class pool_allocator
		{
		public:
			typedef T value_type;
			typedef Mutex mutex;

			typedef value_type * pointer;
			typedef const value_type * const_pointer;
			typedef value_type & reference;
			typedef const value_type & const_reference;
			typedef std::size_t size_type;
			typedef std::ptrdiff_t difference_type;

			template <typename U>
			struct rebind
			{
				typedef pool_allocator<U,  Mutex> other;
			};

		public:
			pool_allocator() { }

			// default copy constructor

			// default assignment operator

			// not explicit, mimicking std::allocator [20.4.1]
			template <typename U>
			pool_allocator(const pool_allocator<U, Mutex> &)
			{ }

			// default destructor

			static pointer address(reference r)
			{ return &r; }
			static const_pointer address(const_reference s)
			{ return &s; }
			static size_type max_size()
			{ return (std::numeric_limits<size_type>::max)(); }
			static void construct(const pointer ptr, const value_type & t)
			{ new (ptr) T(t); }
			static void destroy(const pointer ptr)
			{
				ptr->~T();
				(void) ptr; // avoid unused variable warning
			}

			bool operator==(const pool_allocator &) const
			{ return true; }
			bool operator!=(const pool_allocator &) const
			{ return false; }

			static pointer allocate(const size_type n)
			{
				const pointer ret = static_cast<pointer>(
					generic_singleton_pool<pool_allocator_tag, Mutex>::alloc(n * sizeof(value_type)) );
				if (ret == 0)
					throw std::bad_alloc();
				return ret;
			}
			static pointer allocate(const size_type n, const void * const)
			{ return allocate(n); }
			static void deallocate(const pointer ptr, const size_type n)
			{
				if (ptr == 0 || n == 0)
					return;
				generic_singleton_pool<pool_allocator_tag, Mutex >::free(ptr, n * sizeof(value_type));
			}
		};


	}
}
#endif