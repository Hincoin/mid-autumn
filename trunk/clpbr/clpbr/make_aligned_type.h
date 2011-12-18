#ifndef _MAKE_ALIGNED_TYPE_H_
#define _MAKE_ALIGNED_TYPE_H_

//lzy: try to make structure aligned
namespace detail
{
	template<typename T,size_t AlignSize>
	struct aligned_type_for_T_impl:public T
	{
		char _this_is_lzy_long_member_name_for_align_pad_[AlignSize - sizeof(T)%AlignSize];
	};

	template<typename T, size_t AlignSize, size_t Remain>
	struct make_aligned_type_impl
	{
		typedef aligned_type_for_T_impl<T,AlignSize> type;
	};

	template<typename T, size_t AlignSize>
	struct make_aligned_type_impl<T,AlignSize, 0>
	{
		typedef T type;
	};
}
template<typename T, size_t AlignSize>
struct make_aligned_type{
	typedef typename detail::make_aligned_type_impl<T,AlignSize,sizeof(T) % AlignSize>::type type;
	static_assert(sizeof(type) % AlignSize == 0,"make_aligned_type failed!");
};


#endif