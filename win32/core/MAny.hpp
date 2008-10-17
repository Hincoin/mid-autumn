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
//but this implementation has more abilities 


//add move semantic
#include "Move.hpp"
#include "ParameterType.hpp"

namespace ma{
namespace details{
	struct many_interface;

	struct many_vtable{
		typedef many_interface interface_type;
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
		static const std::type_info& (const interface_type&)
		{
			return std::typeid(interface_type);
		}
		static void destruct(const interface_type& x)
		{
			self(x).~many_static();
		}
		static interface_type* clone(const interface_type& x,void* storage)
		{
			return ::new(storage) many_static(self(x).object);
		}
		static interface_type* move_clone(const interface_type& x,void* storage)
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
	};
	template<typename T>
	const many_vtable many_static<T>::class_vtable={
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
		
		typedef ma_fixed_allocator<T> allocator_type;
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
		static std::type_info& type_info(const interface_type&){return typeid(T);}
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
		static swap(interface_type& x,interface_type& y)
		{
			return std::swap(self(x).object_ptr,self(y).object_ptr);
		}
		const T& get()const {return object_ptr->data;}
		T& get(){return object_ptr->data;}
	};
	template<typename T>
	const many_vtable many_dynamic<T>::vtable=
	{
		&many_dynamic::destruct,
		&many_dynamic::type_info,
		&many_dynamic::clone,
		&many_dynamic::move_clone,
		&many_dynamic::assign,
		&many_dynamic::swap
	};
}

}
#endif
