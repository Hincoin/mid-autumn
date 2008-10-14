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
			
		}


	};
}

}
#endif
