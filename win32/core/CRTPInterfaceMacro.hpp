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
	~_CLASS_NAME (){}\
public:


#define ADD_CRTP_INTERFACE_TYPEDEF(_TYPE_)\
	typedef typename ClassConfig::_TYPE_ _TYPE_;\

	//typedef typename details::ma_traits::_typedef_traits_##_TYPE_<Derived>::type  _TYPE_;

#define ADD_CRTP_INTERFACE_FUNC(_RET_TYPE,_FUNC_NAME, _PARAM_TYPE_LIST, _PARAM_NAME_LIST, _CONST_NESS)\
	_RET_TYPE _FUNC_NAME _PARAM_TYPE_LIST _CONST_NESS {return (derived()._FUNC_NAME _PARAM_NAME_LIST) ;}

#define ADD_CRTP_TEMPLATE_INTERFACE_FUNC( _TEMPLATE_ARGS, _RET_TYPE,_FUNC_NAME, _PARAM_TYPE_LIST, _PARAM_NAME_LIST, _CONST_NESS)\
   template< _TEMPLATE_ARGS > _RET_TYPE _FUNC_NAME _PARAM_TYPE_LIST _CONST_NESS { return (derived().template <_TEMPLATE_ARGS> _FUNC_NAME _PARAM_NAME_LIST);}

#define END_CRTP_INTERFACE()\
};

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
