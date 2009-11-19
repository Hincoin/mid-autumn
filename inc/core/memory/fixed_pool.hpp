#ifndef _INCLUDED_FIXED_POOL_HPP_
#define _INCLUDED_FIXED_POOL_HPP_


#include <boost/pool/pool.hpp>
#include "details/fixed_pool_impl.hpp"

namespace ma{
	namespace core{
		template<unsigned RequestedSize,unsigned NextSize,typename UserAllocator = details::default_user_allocator_malloc_free>
		struct fixed_pool{
			typedef typename UserAllocator::size_type size_type;
			typedef typename UserAllocator::difference_type difference_type;

			fixed_pool(){}
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
				return memset(alloc(),0,pool_impl_.get_requested_size());
			}
			bool release_memory()
			{
				return pool_impl_.release_memory();
			}
		private:
			//default_fixed_pool_impl<UserAllocator> pool_impl_;
			//fixed_pool_impl_fast_segregate<RequestedSize,NextSize,UserAllocator> pool_impl_;
			fixed_pool_impl_small<RequestedSize,UserAllocator> pool_impl_;
		};
	}
}

#endif
