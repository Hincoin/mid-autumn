#ifndef MA_FIXED_ALLOCATOR_HPP
#define MA_FIXED_ALLOCATOR_HPP

#include "FSBAllocator.hpp"

namespace ma{
	//an interface template for allocators
	//adding move semantic
	template<typename T,typename AllocatorImpl = FSBAllocator<T> >
	class ma_fixed_allocator:private AllocatorImpl{
	public:
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef Ty *pointer;
		typedef const Ty *const_pointer;
		typedef Ty& reference;
		typedef const Ty& const_reference;
		typedef Ty value_type;

		typedef AllocatorImpl implementation;

		pointer address(reference val) const { return implementation::address(val); }
		const_pointer address(const_reference val) const { return implementation::address(val); }

		template<class Other>
		struct rebind
		{
			typedef typename implementation::template rebind<Other>::other other;
		};

		ma_fixed_allocator() throw() {}

		template<class Other,class impl>
		ma_fixed_allocator(const ma_fixed_allocator<Other,impl>&) throw() {}

		template<class Other,class impl>
		ma_fixed_allocator& operator=(const ma_fixed_allocator<Other,impl>&) { return *this; }

		pointer allocate(size_type count, const void* = 0)
		{
			return implementation::allocate(count);
		}

		void deallocate(pointer ptr, size_type sz)
		{
			return implementation::deallocate(ptr,sz);
		}

		void construct(pointer ptr, const Ty& val)
		{
			implementation::construct(ptr,val);
		}

		void destroy(pointer ptr)
		{
			implementation::destroy(ptr);
		}

		size_type max_size() const throw() { return implementation::max_size(); }
	};
}
#endif