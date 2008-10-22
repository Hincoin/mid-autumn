#ifndef MA_CRTPINTERFACEMACRO_HPP
#define MA_CRTPINTERFACEMACRO_HPP

#define BEGIN_CRTP_INTERFACE(_CLASS_NAME)\
	template<typename Derived>\
class _CLASS_NAME{\
	_CLASS_NAME##& derived(){return static_cast<_CLASS_NAME&>(*this);}\
	const _CLASS_NAME##& derived()const{return static_cast<const _CLASS_NAME&>(*this);}\
protected:\
	~##_CLASS_NAME(){}\
public:


#define ADD_CRTP_INTERFACE_TYPEDEF(_TYPE_)\
	typedef typename Derived::_TYPE_ _TYPE_;

#define ADD_CRTP_INTERFACE_FUNC_0(_RET_TYPE,_FUNC_NAME, CONST_NESS)\
	_RET_TYPE _FUNC_NAME() CONST_NESS {return derived().##_FUNC_NAME();}

#define ADD_CRTP_INTERFACE_FUNC_1(_RET_TYPE,_FUNC_NAME, PARAM_TYPE, PARAM_NAME ,CONST_NESS)\
	_RET_TYPE _FUNC_NAME(PARAM_TYPE PARAM_NAME) CONST_NESS {return derived().##_FUNC_NAME(PARAM_NAME);}
//
//#define ADD_CRTP_INTERFACE_FUNC_2(_RET_TYPE,_FUNC_NAME, CONST_NESS)\
//	_RET_TYPE _FUNC_NAME() CONST_NESS {return derived().##_FUNC_NAME();}
//
//#define ADD_CRTP_INTERFACE_FUNC_3(_RET_TYPE,_FUNC_NAME, CONST_NESS)\
//	_RET_TYPE _FUNC_NAME() CONST_NESS {return derived().##_FUNC_NAME();}
//
//#define ADD_CRTP_INTERFACE_FUNC_4(_RET_TYPE,_FUNC_NAME, CONST_NESS)\
//	_RET_TYPE _FUNC_NAME() CONST_NESS {return derived().##_FUNC_NAME();}
//
//#define ADD_CRTP_INTERFACE_FUNC_5(_RET_TYPE,_FUNC_NAME, CONST_NESS)\
//	_RET_TYPE _FUNC_NAME() CONST_NESS {return derived().##_FUNC_NAME();}


#define END_CRTP_INTERFACE()\
};


#endif