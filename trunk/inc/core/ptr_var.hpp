#ifndef _MA_INCLUDED_PTR_VARIANT_HPP_
#define _MA_INCLUDED_PTR_VARIANT_HPP_

#include "boost/preprocessor/cat.hpp"
#include "boost/preprocessor/enum.hpp"
#include "boost/preprocessor/enum_params.hpp"
#include "boost/preprocessor/enum_shifted_params.hpp"
#include "boost/preprocessor/repeat.hpp"

#include "boost/preprocessor/tuple/elem.hpp"

#include "boost/mpl/limits/list.hpp"

#include <boost/type_traits.hpp>
#include <boost/mpl/size.hpp>
#include "PtrTraits.hpp"

#define POINTER_VARIANT_LIMIT_TYPES \
	BOOST_MPL_LIMIT_LIST_SIZE

#define POINTER_VARIANT_ENUM_SHIFTED_PARAMS( param )  \
	BOOST_PP_ENUM_SHIFTED_PARAMS(POINTER_VARIANT_LIMIT_TYPES, param)

#define POINTER_VARIANT_ENUM_PARAMS(param)\
	BOOST_PP_ENUM_PARAMS(POINTER_VARIANT_LIMIT_TYPES,param)\

namespace ma{
	namespace details{
		namespace ptr_var{struct void_;}
	}

#   define POINTER_VARIANT_AUX_DECLARE_PARAMS_IMPL(z, N, T) \
	typename BOOST_PP_CAT(T,N) = details::ptr_var::void_ \

#	define POINTER_VARIANT_AUX_DECLARE_PARAMS \
	BOOST_PP_ENUM( \
	POINTER_VARIANT_LIMIT_TYPES \
	, POINTER_VARIANT_AUX_DECLARE_PARAMS_IMPL \
	, T \
	) \


	template < POINTER_VARIANT_AUX_DECLARE_PARAMS > class ptr_var;

#undef POINTER_VARIANT_AUX_DECLARE_PARAMS_IMPL
#undef POINTER_VARIANT_AUX_DECLARE_PARAMS

	namespace details{
		namespace ptr_var{


			template <typename T>
			struct convert_void
			{
				typedef T type;
			};

			template <>
			struct convert_void< void_ >
			{
				typedef boost::mpl::na type;
			};




			template < POINTER_VARIANT_ENUM_PARAMS(typename T) >
			struct make_variant_list
			{
			public: // metafunction result

				// [Define a macro to convert any void(NN) tags to mpl::void...]
#   define POINTER_VARIANT_AUX_CONVERT_VOID(z, N,_)  \
	typename convert_void< BOOST_PP_CAT(T,N) >::type

				// [...so that the specified types can be passed to mpl::list...]
				typedef typename boost::mpl::list<
					BOOST_PP_ENUM(
					POINTER_VARIANT_LIMIT_TYPES
					, POINTER_VARIANT_AUX_CONVERT_VOID
					, _
					)
				>::type type;

				// [...and, finally, the conversion macro can be undefined:]
#   undef BOOST_VARIANT_AUX_CONVERT_VOID

			};

			template <typename Types>
			struct over_sequence
			{
				typedef Types type;
			};

			template <typename T>
			struct is_over_sequence
				: boost::mpl::false_
			{
			};

			template <typename Types>
			struct is_over_sequence< over_sequence<Types> >
				: boost::mpl::true_
			{
			};

			struct ptr_var_deleter:boost::static_visitor<>{
				template <typename T>
				void operator()(T* x)const
				{
					delete x;
				}
			};
			struct ptr_var_array_deleter:boost::static_visitor<>{
				template<typename T>
				void operator()(T* x)const
				{
					delete []x;
				}
			};

			struct apply_visitor_unrolled {};
			template <typename Iter, typename LastIter>
			struct visitation_impl_step
			{
				typedef typename boost::mpl::deref<Iter>::type type;

				typedef typename boost::mpl::next<Iter>::type next_iter;
				typedef visitation_impl_step<
					next_iter, LastIter
				> next;
			};

			template <typename LastIter>
			struct visitation_impl_step< LastIter,LastIter >
			{
				typedef apply_visitor_unrolled type;
				typedef visitation_impl_step next;
			};


		}

	}

}


//this is a type-safe void* , under some compiler this can be faster than T* pointer, why?
namespace ma{
	template<POINTER_VARIANT_ENUM_PARAMS(typename T)>
	inline void* get_ptr(const ma::ptr_var<POINTER_VARIANT_ENUM_PARAMS(T)>& lhs);
	template<typename T0 ,
		POINTER_VARIANT_ENUM_SHIFTED_PARAMS(typename T) >
	class ptr_var{
		typedef typename
			boost::mpl::if_<details::ptr_var::is_over_sequence<T0>,
			T0,
			typename details::ptr_var::make_variant_list<
			POINTER_VARIANT_ENUM_PARAMS(T)
			> >::type seq_type_wrapper;
		typedef typename seq_type_wrapper::type seq_type;
		void* ptr;
		int which;
		//char aligned_storage[ boost::alignment_of<char[sizeof(void*) + sizeof(char)]>::value ];
	public:
		typedef seq_type types;
		typedef ptr_var<POINTER_VARIANT_ENUM_PARAMS(T)> class_type;
		//////////////////////////////////////////////////////////////////////////
		template<typename U>
		ptr_var(U* x):ptr(x){
			typedef typename boost::mpl::find_if<
				seq_type
				, boost::is_same<
				boost::mpl::_1
				, U
				>
			>::type found_it;

			typedef typename boost::mpl::end<seq_type>::type not_found;

			typedef typename boost::is_same<
				found_it, not_found
			>::type is_foreign_variant;
			//type should compatible
			BOOST_STATIC_ASSERT(!(is_foreign_variant::value));
			which = boost::mpl::distance<typename boost::mpl::begin<seq_type>::type,found_it>::value;
		}

		ptr_var():ptr(0),which(-1){}
		ptr_var(const ptr_var& o):ptr(o.ptr),which(o.which){}
		ptr_var& operator=(ptr_var rhs){this->swap(rhs);return *this;}
		void swap(ptr_var& rhs){std::swap(ptr,rhs.ptr);std::swap(which,rhs.which);}

		//////////////////////////////////////////////////////////////////////////
		int which_type()const{return which;}

		template<typename T>
		T* get(){
			typedef T* result_type;

			typedef typename boost::mpl::find_if<
				seq_type
				, boost::is_same<
				boost::mpl::_1
				, typename boost::remove_const<T>::type
				> // or boost::is_convertible<>
			>::type found_it;

			typedef typename boost::mpl::end<seq_type>::type not_found;

			typedef typename boost::is_same<
				found_it, not_found
			>::type is_foreign_variant;
			//type should compatible
			//BOOST_STATIC_ASSERT((boost::is_pointer<T>::value));
			BOOST_STATIC_ASSERT(( ! is_foreign_variant::value));

			if (which != boost::mpl::distance<typename boost::mpl::begin<seq_type>::type,found_it>::value)
			{
				throw boost::bad_get();
			}
			return static_cast<result_type>(ptr);
		}
		template<typename T>
		const T* get()const{
			typedef T* result_type;

			typedef typename boost::mpl::find_if<
				seq_type
				, boost::is_same<
				boost::mpl::_1
				, typename boost::remove_const<T>::type
				> // or boost::is_convertible<>
			>::type found_it;

			typedef typename boost::mpl::end<seq_type>::type not_found;

			typedef typename boost::is_same<
				found_it, not_found
			>::type is_foreign_variant;
			//type should compatible
			//BOOST_STATIC_ASSERT((boost::is_pointer<T>::value));
			BOOST_STATIC_ASSERT(( ! is_foreign_variant::value));

			if (which != boost::mpl::distance<typename boost::mpl::begin<seq_type>::type,found_it>::value)
			{
				throw boost::bad_get();
			}
			return static_cast<result_type>(ptr);
		}
		//////////////////////////////////////////////////////////////////////////
		template<typename Visitor>
		typename Visitor::result_type
			apply_visitor(Visitor& v)const{
				// Typedef apply_visitor_unrolled steps and associated types...

				typedef typename boost::mpl::begin<seq_type>::type first_iter;
				typedef typename boost::mpl::end<seq_type>::type last_iter;
				typedef typename details::ptr_var::visitation_impl_step<first_iter,last_iter> step0;

#   define POINTER_VARIANT_AUX_APPLY_VISITOR_STEP_TYPEDEF(z, N, _) \
	typedef typename BOOST_PP_CAT(step,N)::type BOOST_PP_CAT(U,N); \
	typedef typename BOOST_PP_CAT(step,N)::next \
	BOOST_PP_CAT(step, BOOST_PP_INC(N)); \
	/**/

				BOOST_PP_REPEAT(
					POINTER_VARIANT_LIMIT_TYPES
					, POINTER_VARIANT_AUX_APPLY_VISITOR_STEP_TYPEDEF
					, _
					)

#   undef POINTER_VARIANT_AUX_APPLY_VISITOR_STEP_TYPEDEF


					typedef typename Visitor::result_type result_type;
				switch(which)
				{
					// ...applying the appropriate case:
#   define POINTER_VARIANT_AUX_APPLY_VISITOR_STEP_CASE(z, N, _) \
	case ((N)): \
	return invoke_visitor<result_type>(v,static_cast<BOOST_PP_CAT(U,N)*>(ptr)); \
	/**/

					BOOST_PP_REPEAT(
						POINTER_VARIANT_LIMIT_TYPES
						, POINTER_VARIANT_AUX_APPLY_VISITOR_STEP_CASE
						, _
						)

#   undef POINTER_VARIANT_AUX_APPLY_VISITOR_STEP_CASE
				}

				assert(false);
				return forced_return<result_type>();
		}

		template<typename Visitor>
		typename Visitor::result_type
			apply_visitor(Visitor& v){
				// Typedef apply_visitor_unrolled steps and associated types...

				typedef typename boost::mpl::begin<seq_type>::type first_iter;
				typedef typename boost::mpl::end<seq_type>::type last_iter;
				typedef typename details::ptr_var::visitation_impl_step<first_iter,last_iter> step0;

#   define POINTER_VARIANT_AUX_APPLY_VISITOR_STEP_TYPEDEF(z, N, _) \
	typedef typename BOOST_PP_CAT(step,N)::type BOOST_PP_CAT(U,N); \
	typedef typename BOOST_PP_CAT(step,N)::next \
	BOOST_PP_CAT(step, BOOST_PP_INC(N)); \
	/**/

				BOOST_PP_REPEAT(
					POINTER_VARIANT_LIMIT_TYPES
					, POINTER_VARIANT_AUX_APPLY_VISITOR_STEP_TYPEDEF
					, _
					)

#   undef POINTER_VARIANT_AUX_APPLY_VISITOR_STEP_TYPEDEF


					typedef typename Visitor::result_type result_type;
				switch(which)
				{
					// ...applying the appropriate case:
#   define POINTER_VARIANT_AUX_APPLY_VISITOR_STEP_CASE(z, N, _) \
	case ((N)): \
	return invoke_visitor<result_type>(v,static_cast<BOOST_PP_CAT(U,N)*>(ptr)); \
	/**/

					BOOST_PP_REPEAT(
						POINTER_VARIANT_LIMIT_TYPES
						, POINTER_VARIANT_AUX_APPLY_VISITOR_STEP_CASE
						, _
						)

#   undef POINTER_VARIANT_AUX_APPLY_VISITOR_STEP_CASE
				}

				assert(false);
				return forced_return<result_type>();
		}
		//////////////////////////////////////////////////////////////////////////

	private:
		//
		template<typename R,typename Visitor,typename T>
		static R invoke_visitor(Visitor& v,T* x)
		{
			typedef typename boost::mpl::find_if<
				seq_type
				, boost::is_same<
				boost::mpl::_1
				, T
				>
			>::type found_it;

			typedef typename boost::mpl::end<seq_type>::type not_found;

			typedef typename boost::is_same<
				found_it, not_found
			>::type is_foreign_variant;

			return invoke_impl<R>(v,x,
				is_foreign_variant());
		}
		template<typename R,typename Visitor,typename T>
		static R invoke_impl(Visitor& v,T* x,const boost::false_type&){
			return v(x);
		};
		template<typename R,typename Visitor,typename T>
		static R invoke_impl(Visitor& v,T* x,const boost::true_type&){
			return forced_return<R>();
		};

		template <typename T>
		inline static T forced_return_impl(const boost::true_type&)
		{
				BOOST_STATIC_ASSERT((boost::is_void<T>::value));
		}
		template <typename T>
		inline static T forced_return_impl(const boost::false_type&)
		{
			assert(false);
			typedef typename boost::remove_reference<T>::type basic_type;
			basic_type* dummy = 0;
			return *(static_cast< basic_type* >(dummy));
		}
		// not optimized implementation
		template <typename T>
		inline static T forced_return(  )
		{
			// logical error: should never be here! (see above)
			return forced_return_impl<T>(boost::is_void<T>());
		}
	public:
		//////////////////////////////////////////////////////////////////////////
		//boolean operations
		bool operator!()const{return ptr == 0;}
		inline friend bool operator== (const class_type& lhs,const void* rhs)
		{
			return lhs.ptr == rhs;
		};
		inline friend bool operator==(const void* lhs,const class_type& rhs)
		{
			return lhs == rhs.ptr;
		}
		inline friend bool operator!=(const class_type& lhs,const void* rhs)
		{
			return lhs.ptr != rhs;
		}
		inline friend bool operator!=(const void* lhs,const class_type& rhs)
		{
			return lhs != rhs.ptr;
		}
		template<class U>
		inline friend bool operator==(const class_type& lhs,const U* rhs){
			return lhs.ptr == rhs;
		}
		template<class U>
		inline friend bool operator==(const U* lhs,const class_type& rhs)
		{
			return lhs == rhs.ptr;
		}
		template<class U>
		inline friend bool operator!=(const class_type& lhs,const U* rhs){
			return lhs.ptr != rhs;
		}
		template<class U>
		inline friend bool operator!=(const U* lhs,const class_type& rhs)
		{
			return lhs != rhs.ptr;
		}
		template<POINTER_VARIANT_ENUM_PARAMS(typename U)>
		bool operator==(const ptr_var<POINTER_VARIANT_ENUM_PARAMS(U)>& rhs)const
		{
			return ptr == rhs.ptr;
		}

		template<POINTER_VARIANT_ENUM_PARAMS(typename U)>
		bool operator!=(const ptr_var<POINTER_VARIANT_ENUM_PARAMS(U)>& rhs)const
		{
			return ptr != rhs.ptr;
		}


	private:
		class _forbid_delete{void operator delete(void*);};
		template<POINTER_VARIANT_ENUM_PARAMS(typename U)>
		friend void* get_ptr(const ma::ptr_var<POINTER_VARIANT_ENUM_PARAMS(U)>& lhs);
	public:
		operator _forbid_delete*()const
		{
			if(!ptr)return 0;
			static _forbid_delete t;
			return &t;
		}
	};


	template<POINTER_VARIANT_ENUM_PARAMS(typename U)>
	inline void* get_ptr(const ma::ptr_var<POINTER_VARIANT_ENUM_PARAMS(U)>& lhs){
		return lhs.ptr;
	}
	template<typename Types>
	struct make_ptr_var_over_sequence{
		typedef ptr_var<details::ptr_var::over_sequence<Types> > type;
	};

}

namespace std{
	template<POINTER_VARIANT_ENUM_PARAMS(typename T)>
	struct less< ma::ptr_var<POINTER_VARIANT_ENUM_PARAMS(T)> >:
		public binary_function<ma::ptr_var<POINTER_VARIANT_ENUM_PARAMS(T)>,
		ma::ptr_var<POINTER_VARIANT_ENUM_PARAMS(T)>,bool>
	{
		bool operator()(const ma::ptr_var<POINTER_VARIANT_ENUM_PARAMS(T)>& lhs,
			const ma::ptr_var<POINTER_VARIANT_ENUM_PARAMS(T)>& rhs)const
		{
			return less<void*>()(ma::get_ptr(lhs),ma::get_ptr(rhs));
		}
	};

}


//////////////////////////////////////////////////////////////////////////
//variant operations

namespace ma{
	template<typename T>
	struct is_ptr_variant:boost::false_type{};
	template<POINTER_VARIANT_ENUM_PARAMS(typename T)>
	struct is_ptr_variant< ptr_var<POINTER_VARIANT_ENUM_PARAMS(T)> >:boost::true_type{};

	template<POINTER_VARIANT_ENUM_PARAMS(typename T)>
	void inline delete_ptr(ptr_var<POINTER_VARIANT_ENUM_PARAMS(T)>& p)
	{
	    details::ptr_var::ptr_var_deleter ptr_var_deleter;
		return p.apply_visitor(ptr_var_deleter);
	}
	template<POINTER_VARIANT_ENUM_PARAMS(typename T)>
	void inline delete_array(ptr_var<POINTER_VARIANT_ENUM_PARAMS(T)>& p)
	{
		return p.apply_visitor(details::ptr_var::ptr_var_array_deleter());
	}

	namespace details{

		template<typename T,typename U>
		struct type_getter{
			typedef typename boost::add_reference<const U>::type const_result_type;
			typedef typename boost::add_reference<U>::type ref_result_type;

			const_result_type operator()(const U& x)const{return x;}
			ref_result_type operator()(U& x)const{return x;}
		};
		template<typename T,BOOST_VARIANT_ENUM_PARAMS(typename T)>
		struct type_getter<T, boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >{
			typedef boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> var_t;
			typedef typename boost::add_reference<const T>::type const_result_type;
			typedef typename boost::add_reference<T>::type ref_result_type;

			const_result_type operator()(const var_t& x)const{return boost::get<const_result_type>(x);}
			ref_result_type operator()(var_t& x)const{return boost::get<ref_result_type>(x);}
		};
		template<typename T,POINTER_VARIANT_ENUM_PARAMS(typename T)>
		struct type_getter<T, ma::ptr_var<POINTER_VARIANT_ENUM_PARAMS(T)> >{
			typedef ma::ptr_var<POINTER_VARIANT_ENUM_PARAMS(T)> var_t;
			typedef typename boost::remove_all_extents<T>::type not_extents;
			typedef typename boost::remove_pointer<typename boost::remove_const<not_extents>::type>::type not_const_pointer;
			typedef typename boost::remove_reference<not_const_pointer>::type bare_type;

			typedef typename boost::mpl::if_<boost::is_pointer<T>,
				typename boost::remove_const<T>::type,
				typename boost::remove_const<typename boost::add_reference<T>::type>::type
			>::type ref_result_type;
			typedef
				typename boost::mpl::if_<boost::is_pointer<T>,
				typename boost::add_const<T>::type,
				typename boost::add_const<typename boost::add_reference<T>::type>::type
				>::type const_result_type;


			const_result_type operator()(const var_t& x)const{return invoke_impl(x,boost::is_pointer<T>());}
			ref_result_type operator()(var_t& x)const{return invoke_impl(x,boost::is_pointer<T>());}

		private:
			ref_result_type invoke_impl(var_t& x,const boost::true_type&)const{return x.template get<bare_type>();}
			ref_result_type invoke_impl(var_t& x,const boost::false_type&)const{return *(x.template get<bare_type>());}
			const_result_type invoke_impl(const var_t& x,const boost::true_type&)const{return x.template get<const bare_type>();}
			const_result_type invoke_impl(const var_t& x,const boost::false_type&)const{return *(x.template get<const bare_type>());}
		};
	}

	//if pointer T return pointer
	//else return const referenct
	template<typename T,typename U>
	inline
		typename boost::mpl::if_<boost::is_pointer<T>,
		typename boost::add_const<T>::type,
		typename boost::add_const<typename boost::add_reference<T>::type>::type
		>::type
		get(const U& x){
			return details::type_getter<T,U>()(x);
	}
	template<typename T,typename U>
	inline
		typename boost::mpl::if_<boost::is_pointer<T>,
		typename boost::remove_const<T>::type,
		typename boost::remove_const<typename boost::add_reference<T>::type>::type
		>::type
		get(U& x){
			return details::type_getter<T,U>()(x);
	}


	namespace details{
		template<typename TypeTag,typename Visitor>
		inline boost::apply_visitor_delayed_t<Visitor> apply_visitor_impl(Visitor& visitor,const boost::true_type&)
		{
			return boost::apply_visitor_delayed_t<Visitor>(visitor);
		}
		template<typename TypeTag,typename Visitor>
		inline Visitor& apply_visitor_impl(Visitor& visitor,const boost::false_type&)
		{
			return visitor;
		}
		template<typename TypeTag,typename Visitor>
		inline boost::apply_visitor_delayed_t<Visitor> apply_visitor_impl(const Visitor& visitor,const boost::true_type&)
		{
			return boost::apply_visitor_delayed_t<Visitor>(const_cast<Visitor&>(visitor));
		}
		template<typename TypeTag,typename Visitor>
		inline const Visitor& apply_visitor_impl(const Visitor& visitor,const boost::false_type&)
		{
			return visitor;
		}
	}
	template <typename TypeTag,typename Visitor>
	inline typename boost::mpl::if_<
		is_ptr_variant< TypeTag >,
		boost::apply_visitor_delayed_t<Visitor>,
		Visitor&>::type
		apply_visitor(Visitor& visitor)
	{
		return details::apply_visitor_impl<TypeTag>(visitor,is_ptr_variant<TypeTag>());
	}

	template <typename TypeTag,typename Visitor>
	inline typename boost::mpl::if_<
		is_ptr_variant< TypeTag >,
		boost::apply_visitor_delayed_t<Visitor>,
		const Visitor&>::type
		apply_visitor(const Visitor& visitor)
	{
		return details::apply_visitor_impl<TypeTag>(visitor,is_ptr_variant<TypeTag>());
	}


#define MAKE_PARAMETER_TYPEDEF(z,n,_) typedef T##n parameter_type##n;

#define MAKE_FUNCTOR_TRAITS_N(z,N,_)\
	template<typename R BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N,typename T)>\
	struct functor_traits##N{\
	typedef R result_type;\
	BOOST_PP_REPEAT(N,MAKE_PARAMETER_TYPEDEF,_)	\
	};\

#define MAKE_FUNCTOR_TRAITS(z,N,_)\
	template<typename R BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N,typename T)>\
	struct functor_traits<R(BOOST_PP_ENUM_PARAMS(N,T))>:functor_traits##N<R BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N,T)>\
	{};\

	template<typename F>
	struct functor_traits;

#define MAX_FUNCTOR_PARAMETER_LIMITS 20
	BOOST_PP_REPEAT(MAX_FUNCTOR_PARAMETER_LIMITS,MAKE_FUNCTOR_TRAITS_N,_)
		BOOST_PP_REPEAT(MAX_FUNCTOR_PARAMETER_LIMITS,MAKE_FUNCTOR_TRAITS,_)

		//all are reference types
#define MAKE_DECL(z,n,P) P##n p##n;
#define MAKE_CONS_ASSIGN(z,n,pp) BOOST_PP_COMMA_IF(n) p##n(pp##n)
#define MAKE_CONST_REF_PARAM(z,n,pp) BOOST_PP_COMMA_IF(n) const P##n& pp##n
#define MAKE_REF_PARAM(z,n,pp) BOOST_PP_COMMA_IF(n) P##n& pp##n
#define MAKE_REF_TYPE(z,n,P) BOOST_PP_COMMA_IF(n) P##n&
#define MAKE_PARAM(z,n,pp) BOOST_PP_COMMA_IF(n) P##n pp##n
#define MAKE_FUNC_PARAM(z,n,pp) BOOST_PP_COMMA_IF(n) typename functor_traits<Func>::parameter_type##n pp##n
#define MAKE_REF_CAST(z,n,P) BOOST_PP_COMMA_IF(n) const_cast<P##n&>(pp##n)

#define PP_ENUM_REF_ARGS(AN,P) BOOST_PP_REPEAT(AN,MAKE_REF_PARAM,pp)

#define PP_ENUM_REF_PARAM(AN,P) BOOST_PP_REPEAT(AN,MAKE_REF_TYPE,P)

#define PP_CAST_TO_REF(AN,P) BOOST_PP_REPEAT(AN,MAKE_REF_CAST,P)
#define MAKE_COLON() :

#define PP_COLON_IF(cond) BOOST_PP_IF(cond, MAKE_COLON, BOOST_PP_EMPTY)()




#define DEF_CALL_MEM_FUNC_REF(FN,AN)\
	template<typename R BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,typename P)>\
	inline FN##_visitor##AN<R BOOST_PP_COMMA_IF(AN) PP_ENUM_REF_PARAM(AN,P) > \
	make_##FN##_visitor_ref( BOOST_PP_REPEAT(AN, MAKE_CONST_REF_PARAM, pp) )\
	{\
	typedef FN##_visitor##AN<R BOOST_PP_COMMA_IF(AN) PP_ENUM_REF_PARAM(AN,P) > visitor_type;\
	return visitor_type(PP_CAST_TO_REF(AN,P));\
	}\
	\
	namespace details{\
	template<typename R,typename VarT BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,typename P)>\
	inline R call_##FN##_visitor_ref_impl(VarT var BOOST_PP_COMMA_IF(AN) BOOST_PP_REPEAT(AN, MAKE_CONST_REF_PARAM,pp) ,const boost::true_type&)\
	{\
	typedef FN##_visitor##AN<R BOOST_PP_COMMA_IF(AN) PP_ENUM_REF_PARAM(AN,P) > visitor_type;\
	typedef typename boost::remove_reference<typename boost::remove_all_extents<VarT>::type>::type type_tag_tmp;\
	typedef typename boost::remove_const<type_tag_tmp>::type type_tag;\
	return apply_visitor<type_tag >(/*(visitor_type&)*/make_##FN##_visitor_ref<R>(BOOST_PP_ENUM_PARAMS(AN,pp)))(var);\
	}\
	template<typename R,typename VarT BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,typename P)>\
	inline R call_##FN##_visitor_ref_impl_call_convention(VarT var BOOST_PP_COMMA_IF(AN) BOOST_PP_REPEAT(AN,MAKE_CONST_REF_PARAM,pp) ,const boost::true_type&)\
	{\
	return var->FN (PP_CAST_TO_REF(AN,P));\
	}\
	template<typename R,typename VarT BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,typename P)>\
	inline R call_##FN##_visitor_ref_impl_call_convention(VarT var BOOST_PP_COMMA_IF(AN) BOOST_PP_REPEAT(AN,MAKE_CONST_REF_PARAM,pp) ,const boost::false_type&)\
	{\
	return var.FN (PP_CAST_TO_REF(AN,P));\
	}\
	template<typename R,typename VarT BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,typename P)>\
	inline R call_##FN##_visitor_ref_impl(VarT var BOOST_PP_COMMA_IF(AN) BOOST_PP_REPEAT(AN,MAKE_CONST_REF_PARAM,pp) ,const boost::false_type&)\
	{\
	return call_##FN##_visitor_ref_impl_call_convention<R,VarT>(var BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,pp) , is_ptr_semantic<VarT>());\
	}\
	}\
	\
	template<typename R, typename ObjT BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,typename P)>\
	inline R FN##_ref(ObjT& obj BOOST_PP_COMMA_IF(AN) BOOST_PP_REPEAT(AN, MAKE_CONST_REF_PARAM, pp) )\
	{\
	return details::call_##FN##_visitor_ref_impl<R,ObjT&>(obj BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,pp),is_ptr_variant<ObjT>() );\
	}\
	template<typename R, typename ObjT BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,typename P)>\
	inline R FN##_ref(const ObjT& obj BOOST_PP_COMMA_IF(AN) BOOST_PP_REPEAT(AN, MAKE_CONST_REF_PARAM, pp) )\
	{\
	return details::call_##FN##_visitor_ref_impl<R,const ObjT&>(obj BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,pp),is_ptr_variant<ObjT>() );\
	}\


#define DEF_CALL_MEM_FUNC(FN,AN)\
	\
	template<typename Func>\
	inline FN##_visitor##AN<typename functor_traits<Func>::result_type BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,typename functor_traits<Func>::parameter_type) > \
	make_##FN##_visitor( BOOST_PP_REPEAT(AN, MAKE_FUNC_PARAM, pp) )\
	{\
	typedef FN##_visitor##AN<typename functor_traits<Func>::result_type BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,typename functor_traits<Func>::parameter_type) > visitor_type;\
	return visitor_type(BOOST_PP_ENUM_PARAMS(AN,pp));\
	}\
	namespace details{\
	template<typename Func, typename ObjT >\
	inline typename functor_traits<Func>::result_type call_##FN##_visitor_impl(ObjT obj BOOST_PP_COMMA_IF(AN) BOOST_PP_REPEAT(AN, MAKE_FUNC_PARAM, pp) ,const boost::true_type& )\
	{\
	typedef FN##_visitor##AN<typename functor_traits<Func>::result_type BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,typename functor_traits<Func>::parameter_type) > visitor_type;\
	typedef typename boost::remove_reference<typename boost::remove_all_extents<ObjT>::type>::type type_tag_tmp;\
	typedef typename boost::remove_const<type_tag_tmp>::type type_tag;\
	return apply_visitor<type_tag>(/*(visitor_type&)*/make_##FN##_visitor<Func>( BOOST_PP_ENUM_PARAMS(AN,pp) ))(obj);\
	}\
	template<typename Func, typename ObjT >\
	inline typename functor_traits<Func>::result_type call_##FN##_visitor_impl_call_convention(ObjT obj BOOST_PP_COMMA_IF(AN) BOOST_PP_REPEAT(AN, MAKE_FUNC_PARAM, pp) ,const boost::true_type& )\
	{\
	return obj->FN(BOOST_PP_ENUM_PARAMS(AN,pp));\
	}\
	template<typename Func, typename ObjT >\
	inline typename functor_traits<Func>::result_type call_##FN##_visitor_impl_call_convention(ObjT obj BOOST_PP_COMMA_IF(AN) BOOST_PP_REPEAT(AN, MAKE_FUNC_PARAM, pp) ,const boost::false_type& )\
	{\
	return obj.FN(BOOST_PP_ENUM_PARAMS(AN,pp));\
	}\
	template<typename Func, typename ObjT >\
	inline typename functor_traits<Func>::result_type call_##FN##_visitor_impl(ObjT obj BOOST_PP_COMMA_IF(AN) BOOST_PP_REPEAT(AN, MAKE_FUNC_PARAM, pp) ,const boost::false_type& )\
	{\
	return call_##FN##_visitor_impl_call_convention<Func,ObjT>(obj BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,pp) , is_ptr_semantic<ObjT>());\
	}\
	}\
	template<typename Func, typename ObjT >\
	inline typename functor_traits<Func>::result_type FN (ObjT& obj BOOST_PP_COMMA_IF(AN) BOOST_PP_REPEAT(AN, MAKE_FUNC_PARAM, pp) ) \
	{	return details::call_##FN##_visitor_impl<Func,ObjT&>(obj BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,pp) , is_ptr_variant<ObjT>());}\
	template<typename Func, typename ObjT >\
	inline typename functor_traits<Func>::result_type FN (const ObjT& obj BOOST_PP_COMMA_IF(AN) BOOST_PP_REPEAT(AN, MAKE_FUNC_PARAM, pp) )\
	{	return details::call_##FN##_visitor_impl<Func,const ObjT&>(obj BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,pp), is_ptr_variant<ObjT>());}\


#define MAKE_VISITOR(FN,AN)\
	template<typename R BOOST_PP_COMMA_IF(AN) BOOST_PP_ENUM_PARAMS(AN,typename P)>\
	struct FN##_visitor##AN:boost::static_visitor<>\
	{\
	typedef R result_type;\
	BOOST_PP_REPEAT(AN,MAKE_DECL,P);\
	FN##_visitor##AN(BOOST_PP_REPEAT(AN, MAKE_PARAM, pp)) PP_COLON_IF(AN) BOOST_PP_REPEAT(AN,MAKE_CONS_ASSIGN,pp){}\
	template<typename T>\
	result_type operator()(T& obj)\
	{\
	return invoke_impl<T&>(obj,is_ptr_semantic<T>());\
	}\
	template<typename T>\
	result_type operator()(const T& obj)\
	{\
	return invoke_impl<const T&>(obj,is_ptr_semantic<T>());\
	}\
	template<typename T>\
	result_type invoke_impl(T obj,const boost::true_type&)\
	{return obj->FN(BOOST_PP_ENUM_PARAMS(AN,p));}\
	template<typename T>\
	result_type invoke_impl(T obj,const boost::false_type&)\
	{return obj.FN(BOOST_PP_ENUM_PARAMS(AN,p));}\
	\
	template<typename T>\
	result_type operator()(T& obj)const\
	{\
	return invoke_impl<T&>(obj,is_ptr_semantic<T>());\
	}\
	template<typename T>\
	result_type operator()(const T& obj)const\
	{\
	return invoke_impl<const T&>(obj,is_ptr_semantic<T>());\
	}\
	template<typename T>\
	result_type invoke_impl(T obj,const boost::true_type&)const\
	{return obj->FN(BOOST_PP_ENUM_PARAMS(AN,p));}\
	template<typename T>\
	result_type invoke_impl(T obj,const boost::false_type&)const\
	{return obj.FN(BOOST_PP_ENUM_PARAMS(AN,p));}\
	\
	};\
	DEF_CALL_MEM_FUNC_REF(FN,AN)\
	DEF_CALL_MEM_FUNC(FN,AN)\

//sample usage

//class sample_object{
//	typedef void return_type;
//	typedef int T0;
//	typedef double T1;
//	typedef float T2;
//	return_type sample_method(T0){}
//	return_type sample_method(T0)const{}
//	return_type sample_method(T0,T1){}
//	return_type sample_method(T0,T1)const{}
//	return_type sample_method(T0,T1,T2){}
//};
//MAKE_VISITOR(sample_method,1);
//MAKE_VISITOR(sample_method,2);
//MAKE_VISITOR(sample_method,3);

		//PTR_VAR_CALL_FUNC(int,return_int,3,(a,b,c))
#define PTR_VAR_CALL_FUNC_AUX_CASE(z,M,FUNC_N)\
	case M: if(M >= size_var_types) throw boost::bad_get();\
	   typedef typename boost::mpl::if_c< (M < size_var_types), \
		boost::mpl::advance<typename boost::mpl::begin<var_types>::type,boost::mpl::int_<M> > ,\
		boost::mpl::begin<var_types>  >::type _##M##_th_type_iter_t;typedef typename _##M##_th_type_iter_t::type _##M##_th_type_iter;\
			   return ptr.get<typename boost::mpl::deref<_##M##_th_type_iter>::type>()-> BOOST_PP_TUPLE_ELEM(2,0,FUNC_N) (BOOST_PP_ENUM_PARAMS(BOOST_PP_TUPLE_ELEM(2,1,FUNC_N),p)); 


#define PTR_VAR_CALL_FUNC(FUNC,N)\
template<typename RET , typename PTR_VAR BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N,typename T)> \
	inline	RET ptr_var_##FUNC(PTR_VAR ptr BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_BINARY_PARAMS(N, T, &p) )\
		{\
			typedef typename PTR_VAR::types var_types;\
			static const int size_var_types = boost::mpl::size<var_types>::value;\
			switch(ptr.which_type()){\
BOOST_PP_REPEAT(POINTER_VARIANT_LIMIT_TYPES,PTR_VAR_CALL_FUNC_AUX_CASE,(FUNC,N) )	\
			}\
			throw boost::bad_get();\
		}
		

#define DECL_FUNC(R,FN,PN)\
		namespace details{\
		PTR_VAR_CALL_FUNC(FN,PN)\
		template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline R FN##impl(const ObjT& obj , boost::false_type BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{ return obj.FN(BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline R FN##impl(ObjT& obj , boost::false_type& BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{ return obj.FN(BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline R FN##impl(const ObjT* obj , boost::false_type BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{ return obj->FN(BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline R FN##impl(ObjT* obj , boost::false_type BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{ return obj->FN(BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline R FN##impl(const ObjT& obj , boost::true_type BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{ return ptr_var_##FN<R>(obj BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline R FN##impl(ObjT& obj , boost::true_type BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{ return ptr_var_##FN<R>(obj BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline R FN##impl(const ObjT* obj , boost::true_type BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{ return ptr_var_##FN<R>(*obj BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline R FN##impl(ObjT* obj , boost::true_type BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{ return ptr_var_##FN<R>(*obj BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
		}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline R FN(const ObjT& obj BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{ return details::FN##impl(obj, is_ptr_variant<ObjT>() BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline R FN##impl(ObjT& obj  BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{ return details::FN##impl(obj, is_ptr_variant<ObjT>() BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline R FN##impl(const ObjT* obj  BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{ return details::FN##impl(obj, is_ptr_variant<ObjT>() BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline R FN##impl(ObjT* obj BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{ return details::FN##impl(obj, is_ptr_variant<ObjT>() BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
	




//nested return type
#define DECL_FUNC_NEST(R,FN,PN)\
		namespace details{\
			PTR_VAR_CALL_FUNC(FN,PN)\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline typename ObjT::R FN##impl(const ObjT& obj , boost::false_type BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{return obj.FN(BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline typename ObjT::R FN##impl(ObjT& obj , boost::false_type BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{return obj.FN(BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline typename ObjT::R FN##impl(const ObjT* obj , boost::false_type BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{return obj->FN(BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline typename ObjT::R FN##impl(ObjT* obj , boost::false_type BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{return obj->FN(BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline typename ObjT::R FN##impl(const ObjT& obj , boost::true_type BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{return ptr_var_##FN<typename ObjT::R>(obj BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline typename ObjT::R FN##impl(ObjT& obj , boost::true_type BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{return ptr_var_##FN<typename ObjT::R>(obj BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline typename ObjT::R FN##impl(const ObjT* obj , boost::true_type BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{return ptr_var_##FN<typename ObjT::R>(*obj BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline typename ObjT::R FN##impl(ObjT* obj , boost::true_type BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{return ptr_var_##FN<typename ObjT::R>(*obj BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
	}\
template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline typename ObjT::R FN(const ObjT& obj  BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{return details::FN##impl(obj, is_ptr_variant<ObjT>() BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline typename ObjT::R FN(ObjT& obj  BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{return details::FN##impl(obj, is_ptr_variant<ObjT>() BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline typename ObjT::R FN(const ObjT* obj  BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{return details::FN##impl(obj, is_ptr_variant<ObjT>() BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
	template<typename ObjT BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,typename T)>\
	inline typename ObjT::R FN(ObjT* obj BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_BINARY_PARAMS(PN,T,p) )\
	{return details::FN##impl(obj, is_ptr_variant<ObjT>() BOOST_PP_COMMA_IF(PN) BOOST_PP_ENUM_PARAMS(PN,p));}\
		

}



//#undef MAKE_DECL
//#undef MAKE_CONS_ASSIGN
//#undef MAKE_CONST_REF_PARAM
//#undef MAKE_REF_PARAM
//#undef MAKE_PARAM
//#undef MAKE_FUNC_PARAM
//#undef MAKE_REF_CAST
//#undef PP_ENUM_REF_ARGS
//#undef PP_ENUM_REF_PARAM
//#undef PP_CAST_TO_REF
//#undef MAKE_COLON
//#undef PP_COLON_IF
//#undef DEF_CALL_MEM_FUNC_REF
//#undef DEF_CALL_MEM_FUNC

//template<typename T>
//void delete_ptr()

#endif
