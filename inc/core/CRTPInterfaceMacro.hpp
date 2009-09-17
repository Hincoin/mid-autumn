#ifndef MA_CRTPINTERFACEMACRO_HPP
#define MA_CRTPINTERFACEMACRO_HPP


#define MA_CAT(X,Y) X##Y

#include "NullType.hpp"

#define BEGIN_CRTP_INTERFACE(_CLASS_NAME)\
	template<typename Derived, typename ClassConfig = NullType >\
class _CLASS_NAME{\
	Derived& derived(){return static_cast< Derived& >(*this);}\
	const Derived& derived()const{return static_cast<const Derived& >(*this);}\
protected:\
	/*~_CLASS_NAME (){}\*/\
public:

#define ADD_SAME_TYPEDEF(_CONF_,_TYPE_)\
	typedef typename _CONF_::_TYPE_ _TYPE_;\


#define ADD_CRTP_INTERFACE_TYPEDEF(_TYPE_)\
	ADD_SAME_TYPEDEF(ClassConfig,_TYPE_)\


	//typedef typename details::ma_traits::_typedef_traits_##_TYPE_<Derived>::type  _TYPE_;

#define ADD_CRTP_INTERFACE_FUNC(_RET_TYPE,_FUNC_NAME, _PARAM_TYPE_LIST, _PARAM_NAME_LIST, _CONST_NESS)\
	_RET_TYPE _FUNC_NAME _PARAM_TYPE_LIST _CONST_NESS {return (derived()._FUNC_NAME _PARAM_NAME_LIST) ;}

#define ADD_CRTP_TEMPLATE_INTERFACE_FUNC( _TEMPLATE_ARGS, _RET_TYPE,_FUNC_NAME, _PARAM_TYPE_LIST, _PARAM_NAME_LIST, _CONST_NESS)\
   template< _TEMPLATE_ARGS > _RET_TYPE _FUNC_NAME _PARAM_TYPE_LIST _CONST_NESS { return (derived().template <_TEMPLATE_ARGS> _FUNC_NAME _PARAM_NAME_LIST);}



#include <boost/preprocessor/facilities/expand.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>

#define __CRTP_PROCESS_ARG(R,PREFIX,I,ARG)\
	BOOST_PP_COMMA_IF(I)\
	BOOST_PP_EXPAND(\
			BOOST_PP_CAT(BOOST_PP_CAT(__CRTP_,PREFIX),ARG)\
		       )\

#define __CRTP_PROCESS_ARGS(PREFIX,ARGS)\
	BOOST_PP_SEQ_FOR_EACH_I(\
			__CRTP_PROCESS_ARG,\
			PREFIX,\
			ARGS\
			)\

#define __CRTP_FUNCTION(HAS_RETURN,RT,FN,ARGS,_IS_CONST)\
	RT FN(__CRTP_PROCESS_ARGS(DECL_,ARGS))_IS_CONST\
	{return derived().FN##Impl(__CRTP_PROCESS_ARGS(CALL_,ARGS));}\


#include <boost/preprocessor/tuple/to_seq.hpp>
	//sample usage: CRTP_CONST_VOID_METHOD(double,compute,0,())
	//sample usage: CRTP_CONST_VOID_METHOD(int,ret_int_func,2,(IN(const char*,some_str),IN(some_type&,some_name))
	
#define DECLARE_METHOD(RT,FN,AN,AL,CONSTNESS)\
	RT FN(__CRTP_PROCESS_ARGS(DECL_,BOOST_PP_TUPLE_TO_SEQ(AN,AL)))CONSTNESS;

#define DECLARE_CONST_METHOD(RT,FN,AN,AL)\
	DECLARE_METHOD(RT,FN,AN,AL,const)

#define CRTP_METHOD(RET,FN,AN,AL)\
	__CRTP_FUNCTION(RET,\
			RET,\
			FN,\
			BOOST_PP_TUPLE_TO_SEQ(AN,AL),\
			\
		       )\

#define CRTP_CONST_METHOD(RET,FN,AN,AL)\
	__CRTP_FUNCTION(RET,\
			RET,\
			FN,\
			BOOST_PP_TUPLE_TO_SEQ(AN,AL),\
			const\
		       )\

#define  CRTP_CONST_VOID_METHOD(FN,AN,AL)\
	CRTP_CONST_METHOD(void,FN,AN,AL)\

#include <boost/preprocessor/repetition/enum_params.hpp>

#define __CRTP_TEMPLATE_FUNCTION_EXPANDED(TARGN,RET,FN,ARGS,_IS_CONST)\
	template< BOOST_PP_ENUM_PARAMS(TARGN,typename T) >\
	RET FN(__CRTP_PROCESS_ARGS(DECL_,ARGS))_IS_CONST\
	{return derived().template FN##Impl<BOOST_PP_ENUM_PARAMS(TARGN, T) >(__CRTP_PROCESS_ARGS(CALL_,ARGS));}

#define __CRTP_TEMPLATE_FUNCTION(TARGN,RET,FN,ARGN,ARGS,_IS_CONST)\
__CRTP_TEMPLATE_FUNCTION_EXPANDED(TARGN,RET,FN,BOOST_PP_TUPLE_TO_SEQ(ARGN,ARGS),_IS_CONST)

#define CRTP_TEMPLATE_METHOD(TARGN,RET,FN,ARGN,ARGV)\
	__CRTP_TEMPLATE_FUNCTION(TARGN,RET,FN,ARGN,ARGV,)

#define CRTP_CONST_TEMPLATE_METHOD(TARGN,RET,FN,ARGN,ARGV)\
	__CRTP_TEMPLATE_FUNCTION(TARGN,RET,FN,ARGN,ARGV,const)

#define CRTP_CONST_VOID_TEMPLATE_METHOD(TARGN,FN,ARGN,ARGV)\
	__CRTP_TEMPLATE_FUNCTION(TARGN,void,FN,ARGN,ARGV,)

#define __CRTP_DECL_IN(TYPE,NAME)TYPE NAME

#define __CRTP_CALL_IN(TYPE,NAME) NAME

#define END_CRTP_INTERFACE };

/*
//#define MA_DECLARE_TYPEDEF_TRAITS_TYPE(_TYPE_) \
//	namespace details { namespace ma_traits{\
//	template<typename U> struct _typedef_traits_##_TYPE_;} }
//
//#define MA_SPECIALIZE_TYPEDEF_TRAITS_TYPE(_TYPE_, _CLASS_NAME , _CONFIG_CLASS_NAME)\
//	namespace details {namespace ma_traits{\
//	template<> \
//	struct _typedef_traits_##_TYPE_ < _CLASS_NAME >\
//	{\
//	typedef _CONFIG_CLASS_NAME::_TYPE_ type;\
//	};}}
*/

#endif
