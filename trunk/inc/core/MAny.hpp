#ifndef MA_ANY_HPP
#define MA_ANY_HPP


#include <typeinfo>

#include <algorithm>
#include <typeinfo>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/noncopyable.hpp>
#include <boost/operators.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/has_nothrow_copy.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/type_traits/remove_reference.hpp>


//improved boost::any by luozhiyuan
//for more details see: An Efficient Variant Type -- by Christopher Diggins
//and this implementation has more abilities than adobe's any_regular_t in their open source library


//add move semantic
#include "Move.hpp"
#include "ParameterType.hpp"
#include "TypeConversion.hpp"
#include "FixedAllocator.hpp"

namespace ma{
namespace details{
	struct many_interface;

	struct many_vtable{
		typedef many_interface interface_type;
		//conversion plays
		const void* (*static_cast_func)(const interface_type&,const TypeInfo&);
		void (*destruct)(const interface_type&);
		const std::type_info&         (*type_info)(const interface_type&);
		interface_type*     (*clone)(const interface_type&, void*);
		interface_type*     (*move_clone)(interface_type&, void*);
		void                (*assign)(interface_type&, const interface_type&);
		void                (*swap)(interface_type&, interface_type&);
	};
	union pad_many_vtable{
		const many_vtable* vtable;
			double pad;
	};
	struct many_interface{
		typedef many_interface interface_type;

		pad_many_vtable table;

		many_interface(const many_vtable& x){table.vtable = &x;}

		const void* static_cast_func(const TypeInfo& t_info)const{return table.vtable->static_cast_func(*this,t_info);}
		void destruct()const{return table.vtable->destruct(*this);}
		const std::type_info& type_info()const{return table.vtable->type_info(*this);}
		interface_type* clone(void* x)const{return table.vtable->clone(*this,x);}
		interface_type* move_clone(void* x){return table.vtable->move_clone(*this,x);}
		void assign(const interface_type& x){table.vtable->assign(*this,x);}
		void swap(interface_type& x){table.vtable->swap(*this,x);}
	};

	template<typename T>
	struct many_static :many_interface,boost::noncopyable
	{
		typedef many_interface interface_type;

		T object;
		static const many_vtable class_vtable;

		many_static():interface_type(class_vtable),object(){}

		template<typename U>
		explicit many_static(U x,typename copy_sink<U,T>::type = 0)
			:interface_type(class_vtable),object(x){}
		template<typename U>
		explicit many_static(U x,typename move_sink<U,T>::type  = 0 )
			:interface_type(class_vtable),object( move(x)){}

		static const many_static& self(const interface_type& x)
		{
			return static_cast<const many_static&>(x);
		}
		static many_static& self(interface_type& x)
		{
			return static_cast<many_static&>(x);
		}
		static const std::type_info& type_info(const interface_type&)
		{
			return typeid(interface_type);
		}
		static void destruct(const interface_type& x)
		{
			self(x).~many_static();
		}
		static interface_type* clone(const interface_type& x,void* storage)
		{
			return ::new(storage) many_static(self(x).object);
		}
		static interface_type* move_clone(interface_type& x,void* storage)
		{
			return ::new(storage)many_static(move(self(x).object));
		}
		static void assign(interface_type& x,const interface_type& y)
		{
			self(x).object = self(y).object;
		}
		static void swap(interface_type& x,interface_type& y)
		{
			using std::swap;
			swap(self(x).object,self(y).object);
		}

		const T& get()const{return object;}
		T& get(){return object;}
		static const void* static_cast_func(const interface_type& x,const TypeInfo& t_info)
		{
			//return cast_types<T>::cast_to(t_info,&(self(x).get()));
			return static_cast_func_impl(x,t_info, boost::mpl::bool_<boost::is_pointer<T>::value >());
		}
	private:
		static const void* static_cast_func_impl(const interface_type& x,const TypeInfo& t_info,boost::mpl::bool_<true>)
		{
			return cast_types<typename boost::remove_pointer<T>::type>::cast_to(t_info,(self(x).get()));
		}
		static const void* static_cast_func_impl(const interface_type& x,const TypeInfo& t_info,boost::mpl::bool_<false>)
		{
			return cast_types<T>::cast_to(t_info,&(self(x).get()));
		}


	};
	template<typename T>
	const many_vtable many_static<T>::class_vtable={
		&many_static::static_cast_func,
		&many_static::destruct,
		&many_static::type_info,
		&many_static::clone,
		&many_static::move_clone,
		&many_static::assign,
		&many_static::swap
	};

	template<typename T>
	struct many_dynamic:many_interface,boost::noncopyable{
		typedef many_interface interface_type;
		struct stored_object;

		typedef ma::ma_fixed_allocator<stored_object> allocator_type;
		struct stored_object:boost::noncopyable{
			allocator_type object_allocator;
			T data;
		};
		static stored_object* new_copy(const T& x)
		{
			allocator_type a;
			stored_object* result = a.allocate(1);
			::new (static_cast<void*>(&result->object_allocator)) allocator_type(a);
			try
			{
				::new (static_cast<void*>(&result->data)) T(x);
			}
			catch (...)
			{
				result->object_allocator.~allocator_type();
				a.deallocate(result,1);
			}
			return result;
		}
		static stored_object* new_move(T& x)
		{
			allocator_type a;
			stored_object* result = a.allocate(1);
			::new (&result->object_allocator) allocator_type(a);
			try
			{
				move_construct(&result->data,x);
			}
			catch (...)
			{
				result->object_allocator.~allocator_type();
				a.deallocate(result,1);
			}
			return result;
		}
		stored_object* object_ptr;
		static const many_vtable vtable;

		template<typename U>
		explicit many_dynamic(const U& x,typename copy_sink<U,T>::type = 0)
			:interface_type(vtable),object_ptr(new_copy(x)){}
		template<typename U>
		explicit many_dynamic(U x,typename move_sink<U,T>::type=0)
			:interface_type(vtable),object_ptr(new_move(x)){}

		many_dynamic(move_from<many_dynamic> x)
			: interface_type(vtable),object_ptr(x.source.object_ptr){x.source.object_ptr = 0;}
		many_dynamic():interface_type(vtable),object_ptr(0){}
		~many_dynamic(){
			if (object_ptr)
			{
				allocator_type a = object_ptr->object_allocator;
				object_ptr->object_allocator.~allocator_type();
				object_ptr->data.~T();
				a.deallocate(object_ptr,1);
			}
		}
		static const many_dynamic& self(const interface_type& x){return static_cast<const many_dynamic&>(x);}
		static many_dynamic& self(interface_type& x){return static_cast<many_dynamic&>(x);}
		static const std::type_info& type_info(const interface_type&){return typeid(T);}
		static void destruct(const interface_type& x){return self(x).~many_dynamic();}
		static interface_type* clone(const interface_type& x,void* storage)
		{
			return ::new(storage) many_dynamic(self(x).get());
		}
		static interface_type* move_clone(interface_type& x,void* storage)
		{
			return ::new(storage) many_dynamic(move_from<many_dynamic>(self(x)));
		}
		static void assign(interface_type& x,const interface_type& y)
		{
			self(x).get() = self(y).get();
		}
		static void swap(interface_type& x,interface_type& y)
		{
			return std::swap(self(x).object_ptr,self(y).object_ptr);
		}

		const T& get()const {return object_ptr->data;}
		T& get(){return object_ptr->data;}
		static const void* static_cast_func(const interface_type& x,const TypeInfo& t_info)
		{
			//return cast_types<T>::cast_to(t_info,&(self(x).get()));
			return static_cast_func_impl(x,t_info, boost::mpl::bool_<boost::is_pointer<T>::value >());
		}
	private:
		static const void* static_cast_func_impl(const interface_type& x,const TypeInfo& t_info,boost::mpl::bool_<true>)
		{
			return cast_types<typename boost::remove_pointer<T>::type>::cast_to(t_info,(self(x).get()));
		}
		static const void* static_cast_func_impl(const interface_type& x,const TypeInfo& t_info,boost::mpl::bool_<false>)
		{
			return cast_types<T>::cast_to(t_info,&(self(x).get()));
		}

	};
	template<typename T>
	const many_vtable many_dynamic<T>::vtable=
	{
		&many_dynamic::static_cast_func,
		&many_dynamic::destruct,
		&many_dynamic::type_info,
		&many_dynamic::clone,
		&many_dynamic::move_clone,
		&many_dynamic::assign,
		&many_dynamic::swap
	};
}

class MAny{
	typedef details::many_interface interface_type;
	typedef double storage_type[2];

	//sweet traits and my main work
	template<typename T>
	struct many_traits{
		typedef typename cast_types<T>::stored_type stored_type;
		typedef stored_type& reference_type;
		typedef const stored_type& const_reference_type;
		typedef details::many_static<stored_type> many_static_type;
		typedef details::many_dynamic<stored_type> many_dynamic_type;

		typedef boost::mpl::bool_<((sizeof(many_static_type) <= sizeof(storage_type)) && (boost::has_nothrow_copy<stored_type>::value || is_movable<stored_type>::value))>	use_stack_storage;
		typedef typename boost::mpl::if_<use_stack_storage,
			many_static_type,
			many_dynamic_type>::type many_storage_type;

		typedef typename boost::mpl::if_c<
			boost::is_same<stored_type,T>::value || boost::is_class<T>::value,
			reference_type,T>::type result_type;

		typedef typename boost::mpl::if_c<
			boost::is_same<stored_type,T>::value || boost::is_class<T>::value,
			const_reference_type,T>::type const_result_type;
	};
	template<typename T>
	struct many_traits<T*>{
		typedef typename cast_types<T>::stored_type* stored_type;
		typedef details::many_static<stored_type> many_static_type;
		typedef many_static_type many_storage_type;

		typedef stored_type result_type;
		typedef const typename cast_types<T>::stored_type* const_result_type;

	};

	template<typename T> struct helper;
	template<typename T> friend struct helper;
	struct empty_type{
		friend inline bool operator == (const empty_type&, const empty_type&) { return true; }
		friend inline bool operator < (const empty_type&, const empty_type&) { return false; }
		friend inline void swap(empty_type&, empty_type&) { }
	};
private:
	storage_type data;


	void* storage(){return &data;}
	const void* storage()const{return &data;}

	interface_type& object(){return *static_cast<interface_type*>(storage());}
	const interface_type& object()const{return *static_cast<const interface_type*>(storage());}

	MAny(move_from<interface_type> x){x.source.move_clone(storage());}
public:
	MAny(){ ::new (storage()) many_traits<empty_type>::many_storage_type();}
	MAny(const MAny& other){other.object().clone(storage());}

	MAny(move_from<MAny> other){other.source.object().move_clone(storage());}
	MAny& operator=(MAny other){
		object().destruct();
		other.object().move_clone(storage());
		return *this;
	}
	~MAny(){object().destruct();}

	friend inline void swap(MAny& lhs,MAny& rhs){
		interface_type& a(lhs.object());
		interface_type& b(rhs.object());
		if (a.type_info() == a.type_info())
		{
			a.swap(b);return;
		}
		MAny tmp((move_from<interface_type>(a)));
		a.destruct();

		b.move_clone(lhs.storage());
		b.destruct();

		tmp.object().move_clone(rhs.storage());
	}

	template<typename T>
	explicit MAny(const T& x,typename copy_sink<T>::type = 0){
		::new (storage()) typename many_traits<T>::many_storage_type(x);
	}
	template<typename T>
	explicit MAny(T x,typename move_sink<T>::type = 0)
	{
		::new (storage()) typename many_traits<T>::many_storage_type(move(x));
	}

	template<typename T>
	bool cast(T& x)const
	{
		return helper<T>::cast(*this,x);
	}
	template<typename T>
	typename many_traits<typename boost::remove_reference<T>::type>::const_result_type cast()const
	{
		typedef typename boost::remove_reference<T>::type passed_type;
		return helper<passed_type>::cast(*this);
	}
	template<typename T>
	typename many_traits<typename boost::remove_reference<T>::type>::result_type cast(){
		typedef typename boost::remove_reference<T>::type passed_type;
		return helper<passed_type>::cast(*this);
	}

	template<typename T>
	MAny& assign(T x,typename move_sink<T>::type =0){
		object().destruct();
		::new(storage()) typename many_traits<T>::many_storage_type(move(x));
		return *this;
	}
	template<typename T>
	MAny& assign(const T& other,typename copy_sink<T>::type = 0){
		return helper<T>::assign(*this,other);
	}
	MAny& assign(MAny other){
		object().destruct();
		other.object().move_clone(storage());
		return *this;
	}

	const std::type_info& type_info()const{return object().type_info();}

	template<typename T>
	struct transform{
		typedef typename many_traits<T>::stored_type result_type;

		result_type operator()(MAny& x)const{return x.cast<T>();}

		typename many_traits<T>::const_result_type operator()(const MAny& x)const{return x.cast<T>();}

	};
};

template<typename T>
struct MAny::helper{
	static inline bool cast(const MAny& x,T& y){
		typedef typename many_traits<T>::stored_type stored_type;
		typedef typename many_traits<T>::result_type result_type;

		if(x.type_info() == typeid(stored_type))
		{
			return static_cast<result_type>(
				reinterpret_cast<typename many_traits<T>::many_storage_type&>(x.object()).get());
		}
		void* casted =const_cast<void*>( x.object().static_cast_func(typeid(stored_type)));
		if (!casted)
		{
			return false;
		}
		y = static_cast<T&>(*(stored_type*)(casted));

		return true;
	}
	static inline typename  many_traits<T>::result_type cast(MAny& x)
	{
		typedef typename many_traits<T>::stored_type stored_type;
		typedef typename many_traits<T>::result_type result_type;

		if(x.type_info() == typeid(stored_type))
		{
			return static_cast<result_type>(
				reinterpret_cast<typename many_traits<T>::many_storage_type&>(x.object()).get());
		}
		void* casted =const_cast<void*>( x.object().static_cast_func(typeid(stored_type)));
		if (!casted)
		{
			//std::string cast_info(x.type_info().name());
			//cast_info += " cannot cast to ";
			//cast_info += typeid(stored_type).name();
			throw std::bad_cast(/*cast_info.c_str()*/);
		}
		return static_cast<result_type>(*(stored_type*)(casted));
	}
	static inline typename  many_traits<T>::const_result_type cast(const MAny& x)
	{
		typedef  typename many_traits<T>::stored_type stored_type;
		typedef typename many_traits<T>::const_result_type const_result_type;

		if (x.type_info() == typeid(stored_type))
		{
			return static_cast<const_result_type>(
				reinterpret_cast<const typename many_traits<T>::many_storage_type&>(x.object()).get());
		}
		const void* casted =  x.object().static_cast_func(typeid(stored_type));
		if (!casted)
		{
			//std::string cast_info(x.type_info().name());
			//cast_info += " cannot cast to ";
			//cast_info += typeid(stored_type).name();
			throw std::bad_cast(/*cast_info.c_str()*/);
		}
		return static_cast<const_result_type>(*(const stored_type*)(casted));
	}

	static inline MAny& assign(MAny& lhs,const T& x)
	{
		typedef typename cast_types<T>::stored_type stored_type;
		if (lhs.type_info() == typeid(stored_type))
		{
			lhs.cast<stored_type&>() = static_cast<stored_type>(x);
		}
		else
		{
			swap(lhs,MAny(x));
		}
		return lhs;
	}
};

template<typename T>
struct MAny::helper<T*>{
	static inline bool cast(const MAny& x,T* & y){
		typedef typename many_traits<T>::stored_type data_type;
		typedef typename many_traits<T>::stored_type* stored_type;
		typedef typename many_traits<T>::result_type result_type;

		if(x.type_info() == typeid(stored_type))
		{
			return static_cast<result_type>(
				reinterpret_cast<typename many_traits<T*>::many_storage_type&>(x.object()).get());
		}
		void* casted =const_cast<void*>( x.object().static_cast_func(typeid(data_type)));
		if (!casted)
		{
			return false;
		}
		y = static_cast<T*>((data_type*)(casted));

		return true;
	}
	static inline typename  many_traits<T*>::result_type cast(MAny& x)
	{
		typedef typename many_traits<T*>::stored_type stored_type;
		typedef typename many_traits<T*>::result_type result_type;
		typedef typename many_traits<T>::stored_type data_type;

		if(x.type_info() == typeid(stored_type))
		{
			return static_cast<result_type>(
				reinterpret_cast<typename many_traits<T*>::many_storage_type&>(x.object()).get());
		}
		void* casted =const_cast<void*>( x.object().static_cast_func(typeid(data_type)));
		if (!casted)
		{
			//std::string cast_info(x.type_info().name());
			//cast_info += " cannot cast to ";
			//cast_info += typeid(stored_type).name();
			throw std::bad_cast(/*cast_info.c_str()*/);
		}
		return static_cast<result_type>((stored_type)(casted));
	}
	static inline typename  many_traits<T*>::const_result_type cast(const MAny& x)
	{
		typedef typename many_traits<T*>::stored_type stored_type;
		typedef typename many_traits<T*>::const_result_type const_result_type;
		typedef typename many_traits<T>::stored_type data_type;


		if (x.type_info() == typeid(stored_type))
		{
			return static_cast<const_result_type>(
				reinterpret_cast<const typename many_traits<T*>::many_storage_type&>(x.object()).get());
		}
		const void* casted =  x.object().static_cast_func(typeid(data_type));
		if (!casted)
		{
			//std::string cast_info(x.type_info().name());
			//cast_info += " cannot cast to ";
			//cast_info += typeid(stored_type).name();
			throw std::bad_cast(/*cast_info.c_str()*/);
		}
		return static_cast<const_result_type>((const stored_type)(casted));
	}

	static inline MAny& assign(MAny& lhs,const T* x)
	{
		typedef typename cast_types<T*>::stored_type stored_type;
		if (lhs.type_info() == typeid(stored_type))
		{
			lhs.cast<stored_type&>() = static_cast<stored_type>(x);
		}
		else
		{
			swap(lhs,MAny(x));
		}
		return lhs;
	}
};
}
#endif
