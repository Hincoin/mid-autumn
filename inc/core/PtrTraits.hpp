#ifndef MA_PTRTRAITS_HPP
#define MA_PTRTRAITS_HPP

#include <memory>
#include <functional>

#include <boost/shared_ptr.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/remove_all_extents.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/integral_constant.hpp>

namespace ma
{

	template<typename T>
	struct ptr_deleter;
	template<typename T>
	struct ptr_deleter<T*>:std::unary_function<T*,void>
	{
		void operator()(T* x)const throw()
		{
			delete x;
		}
	};
	template<typename T>
	struct ptr_deleter<T* const>:std::unary_function<T*,void>
	{
		void operator()(T* x)const throw()
		{
			delete x;
		}
	};

	template<typename T>
	struct ptr_deleter<T(*)[]>:std::unary_function<T*,void>
	{
		void operator()(T* x)const throw()
		{
			delete []x;
		}
	};
	template<typename T>
	struct ptr_deleter<T(* const)[]>:std::unary_function<T*,void>
	{
		void operator()(T* x)const throw()
		{
			delete [] x;
		}
	};

	template<typename T>
	struct empty_ptr;

	template<typename T>
	struct empty_ptr<T*>:std::unary_function<T*,void>
	{
		bool operator()(const T* x)const throw()
		{return x == 0;}
	};

	template<typename T>
	struct empty_ptr<T(*)[]>:std::unary_function<T*,void>
	{
		bool operator()(const T* x)const throw()
		{
			return x == 0;
		}
	};



	template<typename T>
	struct ptr_traits;


	template<typename T>
	struct ptr_traits<T (*)[]>
	{
		typedef T value_type;
		typedef T* pointer_type;
		typedef const T* const_pointer_type;
		typedef T& reference_type;
		typedef const T& const_reference_type;
		template<class U> struct rebind{typedef ma::ptr_traits<U> other;};
		enum {is_array = true};
		static void delete_ptr(pointer_type x)throw(){
			ma::ptr_deleter<T(*)[]>()(x);}
		static bool empty_ptr(const_pointer_type x)throw()
		{
			return ma::empty_ptr<T(*)[]>()(x);
		}
	};

	template<typename T>
	struct ptr_traits<T*>
	{
		typedef T value_type;
		typedef T* pointer_type;
		typedef const pointer_type const_pointer_type;
		typedef T& reference_type;
		typedef const T& const_reference_type;

		template<class U> struct rebind{ typedef ma::ptr_traits<U> other;};
		enum {is_array = false};
		static void delete_ptr(pointer_type x)throw(){ma::ptr_deleter<T*>()(x);}
		static bool empty_ptr(const_pointer_type x)throw()
		{	return ma::empty_ptr<T*>()(x);	}
	};

	template<typename T>
	struct ptr_traits<std::auto_ptr<T> >
	{
		typedef typename std::auto_ptr<T>::element_type value_type;
		typedef std::auto_ptr<T> pointer_type;
		typedef std::auto_ptr<const T>	const_pointer_type;
		template<class U> struct rebind{typedef ma::ptr_traits<U> other;};
		enum {is_array = false};
		static void delete_ptr(pointer_type&){}
		static bool empty_ptr(const_pointer_type& x){
			return ma::empty_ptr<T>()(x.get());
		}
	};

	template<typename T>
	struct ptr_traits<boost::shared_ptr<T> >
	{
		typedef typename boost::shared_ptr<T>::element_type value_type;
		typedef boost::shared_ptr<T> pointer_type;
		typedef boost::shared_ptr<const T>	const_pointer_type;
		template<class U> struct rebind{typedef ma::ptr_traits<U> other;};
		enum {is_array = false};
		static void delete_ptr(pointer_type&){}
		static bool empty_ptr(const_pointer_type& x){
			return ma::empty_ptr<T>()(x.get());
		}
	};
	template<typename T>
	struct add_shared_ptr{
		typedef boost::shared_ptr<T> type;
	};
	template<typename Ptr>
	void inline delete_ptr(Ptr p)
	{
		return ptr_traits<Ptr>::delete_ptr(p);
	}
	template<typename I>
	void inline delete_ptr(I b,I e )
	{
		for (;b != e;++b)
		{
			delete_ptr(*b);
		}
	}
	template<typename Ptr>
	void inline delete_array(Ptr p)
	{
		typedef typename ptr_traits<Ptr>::value_type ptr_type;
		return ptr_traits<ptr_type (*)[]>::delete_ptr(p);
	}

	template<typename T>
	struct is_shared_ptr;
	template<typename T>
	struct is_shared_ptr<boost::shared_ptr<T> >:boost::true_type{};

	template<typename T>
	struct is_shared_ptr:boost::false_type{};

	template<typename T>
	struct is_ptr_semantic;
	template<typename T>
	struct is_ptr_semantic:	boost::mpl::if_c<
		boost::is_pointer<typename boost::remove_reference<typename boost::remove_all_extents<T>::type>::type>::value || 
		is_shared_ptr<typename boost::remove_reference<typename boost::remove_all_extents<T>::type>::type>::value,
		boost::true_type,
		boost::false_type
	>::type{};

	template<typename T>
	struct pointer_to_const
	{
		typedef const T* type;
	};
	template<typename T>
	struct pointer_to_const<T*>{
		typedef const T* type;
	};
	template<typename T>
	struct pointer_to_const<const T*>
	{
		typedef const T* type;
	};
	template<typename T>
	struct const_pointer;//T* const

}
#endif
