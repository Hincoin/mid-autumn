#ifndef MA_MOVE_HPP
#define MA_MOVE_HPP


//implement move semantic for c++
//for pimpl and array member
#include <cassert>
#include <iterator>
#include <memory>


#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_class.hpp>
#include <boost/utility/enable_if.hpp>


namespace ma{
	namespace details{

		template<typename T>
		struct class_has_move_assign{
			class type{
				typedef T& (T::*E)(T t);
				typedef char(&no_type)[1];
				typedef char(&yes_type)[2];
				template<E e >struct sfinae
				{typedef yes_type type;};
				template<class U>
				static typename sfinae<&U::operator= >::type test(int);
				template<class U>
				static no_type test(...);
			public:
				enum{value = sizeof(test<T>(1)) == sizeof(yes_type)};
			};
		};

		template<typename T>
		struct has_move_assign:boost::mpl::and_<boost::is_class<T>, class_has_move_assign<T> >{};


		class test_can_convert_anything{};

		template<typename T,typename U>
		struct is_convertible:boost::mpl::or_<
			boost::is_same<T,U> ,
			boost::is_convertible<T,U>
		>{};

	}


	template<typename T>
	struct move_from{
		T& source;
		explicit move_from(const T& x):source(const_cast<T&>(x)){}
		explicit move_from(T& x):source(x){}
		//operator const T&(){return source;}
	};

	template<typename T>
	struct is_movable:boost::mpl::and_<
		boost::is_convertible<move_from<T>,T>,
		details::has_move_assign<T>,
		boost::mpl::not_<boost::is_convertible<details::test_can_convert_anything,T> >
		>
	{};
	template<typename T,
		typename U=T,
		typename R = void*>
	struct copy_sink:boost::enable_if
		< boost::mpl::and_<
		details::is_convertible<T,U>,
		boost::mpl::not_<is_movable<T> >
		>,
		R
		>{};

	template<typename T,
		typename U = T,
		typename R = void*>
	struct move_sink: boost::enable_if<
		boost::mpl::and_<details::is_convertible<T,U>,
		is_movable<T>
		>,
		R
	>{};


	//template<typename T>
	//inline T move(T& x,typename move_sink<T>::type = 0){return T(move_from<T>(x));}

	template<typename T>
	inline move_from<T> move(T& x,typename move_sink<T>::type = 0){return move_from<T>(x);}

	template<typename T>
	inline T& move(T& x,typename copy_sink<T>::type = 0){return x;}

	template<typename T>
	inline move_from<T> force_move(T& x){return move_from<T>(x);}
    template<typename T>
	inline move_from<T> force_move(const T& x){return move_from<T>(x);}

	template<typename InputIterator,typename OutputIterator>
	inline OutputIterator move(InputIterator f,InputIterator l, OutputIterator result)
	{
		while(f!= l){
			*result = move(*f);
			++f;++result;
		}
		return result;
	}

	template<typename InputRange,
		typename OutputRange>
		inline OutputRange move(InputRange& in,OutputRange out)
	{
		return move(boost::begin(in),boost::end(in),out);
	}

	template<typename IBidirIterator,
		typename OBidirIterator>
		inline OBidirIterator
		move_backward(IBidirIterator f,IBidirIterator l,OBidirIterator result)
	{
		while( f != l){
			--l;--result;
			*result = move(*l);
		}
		return result;
	}

	template<typename IBiRange,
		typename OBiRange>
		inline OBiRange move_backward(IBiRange& in,OBiRange out)
	{
		return move_backward(boost::begin(in),boost::end(in),out);
	}

	template <typename Container>
	class back_move_iterator:
		public std::iterator<std::output_iterator_tag,void,void,void,void>
	{
		Container* con_;
	public:
		typedef Container container_type;
		explicit back_move_iterator(Container& x):con_(&x){}
		back_move_iterator& operator=(typename Container::value_type x)
		{	con_->push_back(move(x));return *this;}
		back_move_iterator& operator*(){return *this;}
		back_move_iterator& operator++(){return *this;}
		back_move_iterator& operator++(int){return *this;}
	};

	template<typename Con>
	inline back_move_iterator<Con> back_mover(Con& x)
	{	return back_move_iterator<Con>(x);}

	template<typename T>//T is regular type
	inline void move_construct(T* p,T& x,typename move_sink<T>::type = 0)
	{
		::new (static_cast<void*>(p))T(move_from<T>(x));
	}

	template<typename T>
	inline void move_construct(T* p,const T& x,
		typename copy_sink<T>::type = 0)
	{
		::new (static_cast<void*>(p))T(x);
	}

	template<typename InputIterator,
		typename ForwardIterator>
		inline ForwardIterator
		uninitialized_move(InputIterator f,InputIterator l,ForwardIterator r,typename move_sink<typename std::iterator_traits<InputIterator>::value_type>::type = 0)
	{
		while(f != l){
			move_construct(&*r,*f);
			++f;++r;
		}
		return r;
	}

	template<typename InputIterator,typename ForwardIterator>
	ForwardIterator unintialized_move(
		InputIterator f,InputIterator l,ForwardIterator r,
		typename copy_sink<typename std::iterator_traits<InputIterator>::value_type>::type = 0)
	{
		return std::uninitialized_copy(f,l,r);
	}
}


namespace ma{
	//move swap
	template<typename T>
	void move_swap(T& lhs,T& rhs,  typename ma::move_sink<T>::type = 0)
	{
		T tmp(ma::move(lhs));
		lhs = ma::move(rhs);
		rhs = ma::move(tmp);
	}
	template<typename T>
	void move_swap(T& lhs,T& rhs,  typename ma::copy_sink<T>::type = 0)
	{
		std::swap(lhs,rhs);
	}
}

#endif
