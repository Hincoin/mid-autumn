#ifndef MA_CRTPINTERFACEMACRO_HPP
#define MA_CRTPINTERFACEMACRO_HPP


#define MA_CAT(X,Y) X##Y

#define BEGIN_CRTP_INTERFACE(_CLASS_NAME)\
	template<typename Derived >\
class _CLASS_NAME{\
	Derived& derived(){return static_cast< Derived& >(*this);}\
	const Derived& derived()const{return static_cast<const Derived& >(*this);}\
protected:\
	MA_CAT(~,_CLASS_NAME) (){}\
public:


#define ADD_CRTP_INTERFACE_TYPEDEF(_TYPE_)\
	typedef typename typedef_traits_##_TYPE_<Derived>::type  _TYPE_;

#define ADD_CRTP_INTERFACE_FUNC(_RET_TYPE,_FUNC_NAME, _PARAM_TYPE_LIST, _PARAM_NAME_LIST, _CONST_NESS)\
	_RET_TYPE _FUNC_NAME##_PARAM_TYPE_LIST _CONST_NESS {return MA_CAT(derived(). , _FUNC_NAME##_PARAM_NAME_LIST) ;}


#define END_CRTP_INTERFACE()\
};


#define MA_DECLARE_TYPEDEF_TRAITS_TYPE(_TYPE_) \
	template<typename U> struct typedef_traits_##_TYPE_;

#define MA_SPECIALIZE_TYPEDEF_TRAITS_TYPE(_TYPE_, _CLASS_NAME , _CONFIG_CLASS_NAME)\
	template<> \
	struct typedef_traits_##_TYPE_ < _CLASS_NAME >\
	{\
	typedef _CONFIG_CLASS_NAME::_TYPE_ type;\
	};

#endif