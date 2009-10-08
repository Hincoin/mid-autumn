#ifndef _MA_INCLUDED_POLICYBASEDCLASS_HPP_
#define _MA_INCLUDED_POLICYBASEDCLASS_HPP_


#include <boost/mpl/sequence_tag.hpp>
#include <boost/mpl/iterator_category.hpp>
#include <boost/mpl/iterator_tags.hpp>
#include <boost/mpl/iterator_range.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/equal.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/accumulate.hpp>
#include <boost/type_traits.hpp>

#include <boost/preprocessor.hpp>
namespace ma{

	namespace details{
	namespace policy_based_class
	{
#define TEMPLATE_TEMPLATE_PARAMETER_LIMIT_TYPES \
	BOOST_MPL_LIMIT_LIST_SIZE

#define TEMPLATE_TEMPLATE_PARAMETER_ENUM_SHIFTED_PARAMS( param)\
	BOOST_PP_ENUM_SHIFTED_PARAMS(TEMPLATE_TEMPLATE_PARAMETER_LIMIT_TYPES, param)\

#define TEMPLATE_TEMPLATE_PARAMETER_ENUM_PARAMETERS( param )\
	BOOST_PP_ENUM_PARAMS(TEMPLATE_TEMPLATE_PARAMETER_LIMIT_TYPES,param)\


		template<typename > struct void_;

		template<template<class C> class Configureable>
		struct apply_stub
		{
			struct apply_impl{
				template<typename CC>
				struct apply_derived{
					typedef Configureable<CC> type;
				};
			};

			typedef apply_impl type;
		};
		template<>
		struct apply_stub<void_>
		{
			typedef boost::mpl::na type;
		};

		template<typename C,typename T>
		struct add_config_meta_func{
			typedef typename T::template apply_derived<C>::type type;
		};

		template<TEMPLATE_TEMPLATE_PARAMETER_ENUM_PARAMETERS(template<typename> class T) >
		struct make_template_stub_seq{

			// [Define a macro to convert any void(NN) tags to mpl::void...]
#   define TEMPLATE_TEMPLATE_PARAMERTER_AUX_CONVERT_VOID(z, N,_)  \
	typename apply_stub< BOOST_PP_CAT(T,N) >::type

			// [...so that the specified types can be passed to mpl::list...]
			typedef typename boost::mpl::list< 
				BOOST_PP_ENUM(
				TEMPLATE_TEMPLATE_PARAMETER_LIMIT_TYPES
				, TEMPLATE_TEMPLATE_PARAMERTER_AUX_CONVERT_VOID
				, _
				)
			>::type type;

			// [...and, finally, the conversion macro can be undefined:]
#   undef TEMPLATE_TEMPLATE_PARAMERTER_AUX_CONVERT_VOID
		};

	}
	}

	//forward declarations
#define TEMPLATE_TEMPLATE_PARAMERTER_AUX_DECLARE_PARAMS_IMPL(z,N,T)\
template<typename > class BOOST_PP_CAT(T,N) = details::policy_based_class::void_\

#define TEMPLATE_TEMPLATE_PARAMERTER_AUX_DECLARE_PARAMS \
BOOST_PP_ENUM(\
TEMPLATE_TEMPLATE_PARAMETER_LIMIT_TYPES \
, TEMPLATE_TEMPLATE_PARAMERTER_AUX_DECLARE_PARAMS_IMPL \
, T \
)\

template< TEMPLATE_TEMPLATE_PARAMERTER_AUX_DECLARE_PARAMS > struct template_sequence;
template<typename C, TEMPLATE_TEMPLATE_PARAMERTER_AUX_DECLARE_PARAMS >struct config_template_classes;
template<typename C,template<typename Impl,typename C> class Interface,TEMPLATE_TEMPLATE_PARAMERTER_AUX_DECLARE_PARAMS>
struct class_composer;

#undef TEMPLATE_TEMPLATE_PARAMERTER_AUX_DECLARE_PARAMS_IMPL
#undef TEMPLATE_TEMPLATE_PARAMERTER_AUX_DECLARE_PARAMS


template< template<typename > class T0,
TEMPLATE_TEMPLATE_PARAMETER_ENUM_SHIFTED_PARAMS(template<typename > class T)
>
struct template_sequence{
	typedef typename details::policy_based_class::make_template_stub_seq<TEMPLATE_TEMPLATE_PARAMETER_ENUM_PARAMETERS(T) >::type type;
};

template<typename C, TEMPLATE_TEMPLATE_PARAMETER_ENUM_PARAMETERS(template<typename > class T)>
struct config_template_classes{
private:
	typedef typename template_sequence<TEMPLATE_TEMPLATE_PARAMETER_ENUM_PARAMETERS(T)>::type seq_type;
public:
	typedef typename boost::mpl::accumulate<seq_type,C,
		details::policy_based_class::add_config_meta_func<boost::mpl::_1,boost::mpl::_2> >::type type;
};

	//things go crazy here
	template<typename C,template<typename Impl,typename C> class Interface,
		TEMPLATE_TEMPLATE_PARAMETER_ENUM_PARAMETERS(template<typename > class T)>
	struct class_composer:
		config_template_classes<Interface<class_composer<C,Interface,TEMPLATE_TEMPLATE_PARAMETER_ENUM_PARAMETERS( T )>,C>
		,TEMPLATE_TEMPLATE_PARAMETER_ENUM_PARAMETERS(T)>::type
		{
			//C must be a traits like empty configure class
			BOOST_STATIC_ASSERT((boost::is_empty<C>::value));
	private:
			friend class Interface<
				class_composer<C,Interface,TEMPLATE_TEMPLATE_PARAMETER_ENUM_PARAMETERS( T )>
				,C>;
			typedef Interface<
				class_composer<C,Interface,TEMPLATE_TEMPLATE_PARAMETER_ENUM_PARAMETERS( T )>
				,C> parent_type;
		typedef class_composer< C, Interface,
			TEMPLATE_TEMPLATE_PARAMETER_ENUM_PARAMETERS( T )> class_type;
		typedef typename config_template_classes<C, TEMPLATE_TEMPLATE_PARAMETER_ENUM_PARAMETERS(T) >::type impl_type;
	public:	
		typedef class_type type;
		~class_composer(){
		//make sure interface class is empty
		BOOST_STATIC_ASSERT(sizeof(class_type) == sizeof(impl_type) );
			}
	};

	//template<typename Class>
	//struct policy_adaptor{
	//	template<typename B>
	//	struct type:public B,public Class{};
	//};
	//template<typename Class,template<typename C> class P >
	//struct bind_policy{
	//	template<typename B>
	//	struct type:public P<Class>{};
	//};
}

#endif
