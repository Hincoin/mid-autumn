#ifndef _INCLUDED_FIXED_POOL_HPP_
#define _INCLUDED_FIXED_POOL_HPP_
#include <boost/pool/pool.hpp>
namespace ma{
	namespace core{
		template<typename UserAllocator = boost::default_user_allocator_malloc_free>
		struct fixed_pool{
			typedef typename UserAllocator::size_type size_type;
			explicit fixed_pool(const size_type nrequested_size,
				const size_type nnext_size = 32):pool_impl_(nrequested_size,nnext_size){}
			void* alloc()
			{
				return pool_impl_.malloc();
			}
			void free(void* ptr)
			{
				return pool_impl_.free(ptr);
			}
			void* calloc()
			{
				return memset(malloc(),0,pool_impl_.get_requested_size());
			}
			bool recycle()
			{
				return pool_impl_.release_memory();
			}
		private:
			boost::pool<UserAllocator> pool_impl_;
		};
	}
}

#endif