#ifndef _MA_INCLUDED_OWNPTR_HPP_
#define _MA_INCLUDED_OWNPTR_HPP_

//a pointer which "owns" the pointed object.
namespace ma{
	template<typename T,typename OwnPolicy,typename DestroyPolicy>
	class own_ptr{
		T* ptr;
		mutable bool owned[1];
	public:
		typedef T element_type;
		typedef own_ptr<T,OwnPolicy,DestroyPolicy> self_type;

	};
}


#endif