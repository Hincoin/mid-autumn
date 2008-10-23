#ifndef MA_PTRTRAITS_HPP
#define MA_PTRTRAITS_HPP

#include <memory>
#include <functional>

namespace ma
{

	template<typename T>
	struct delete_ptr;
	template<typename T>
	struct delete_ptr<T*>:std::unary_function<T*,void>
	{
		void operator()(T* x)const throw()
		{
			delete x;
		}
	};
	template<typename T>
	struct delete_ptr<T* const>:std::unary_function<T*,void>
	{
		void operator()(T* x)const throw()
		{
			delete x;
		}
	};

	template<typename T>
	struct delete_ptr<T(*)[]>:std::unary_function<T*,void>
	{
		void operator()(T* x)const throw()
		{
			delete []x;
		}
	};
	template<typename T>
	struct delete_ptr<T(* const)[]>:std::unary_function<T*,void>
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
			ma::delete_ptr<T(*)[]>()(x);}
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
		typedef const reference_type const_reference_type;

		template<class U> struct rebind{ typedef ma::ptr_traits<U> other;};
		enum {is_array = false};
		static void delete_ptr(pointer_type x)throw(){ma::delete_ptr<T*>()(x);}
		static bool empty_ptr(const_pointer_type x)throw()
		{	return ma::empty_ptr<T*>()(x);	}
	};

	template<typename T>
	struct ptr_traits<std::auto_ptr<T> >
	{
		typedef typename std::auto_ptr<T>::element_type value_type;
		typedef std::auto_ptr<T> pointer_type;
		typedef std::auto_ptr<const T>	const_pointer_type;
		template<class U> struct rebind{typedef ma::ptr_traits<U>other;}
		enum {is_array = false};
	};
}
#endif
