#ifndef HASMEMBER_H
#define HASMEMBER_H

//this lib is used as member function query
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>

//M is the number of T::name()'s parameters

#define HAS_MEMFUN_IMPL( z, M, name )							\
template<typename T,typename FuncPtr>										\
struct has_memfun_##name;									\
template<typename T, typename RetType  BOOST_PP_COMMA_IF(M) BOOST_PP_ENUM_PARAMS(M, class T)>		\
struct has_memfun_##name<T, RetType (*) ( BOOST_PP_ENUM_PARAMS(M,T) )  >		\
{							\
	typedef char true_type;	\
	struct false_type		\
	{						\
		true_type dummy[2];	\
	};						\
							\
	template<typename U>	\
	static true_type has_member(U*,RetType (U::*dummy)( BOOST_PP_ENUM_PARAMS(M,T) )=&U::name);	\
	static false_type has_member(void*);	\
											\
	static const bool value=sizeof(has_memfun_##name<T, RetType (*) ( BOOST_PP_ENUM_PARAMS(M,T) )>::has_member((T*)0))==sizeof(true_type);	\
};


#ifndef MA_HAS_MEMFUN_DEFAULT_PARAM_COUNT
#define MA_HAS_MEMFUN_DEFAULT_PARAM_COUNT 20
#endif

#define HAS_MEMFUN( name ) \
	BOOST_PP_REPEAT( MA_HAS_MEMFUN_DEFAULT_PARAM_COUNT , HAS_MEMFUN_IMPL , name ) 

//namespace Detector {
//	typedef char NotFound;
//	struct StaticFunction { NotFound x [2]; };
//	struct NonStaticFunction { NotFound x [3]; };
//	struct StaticData { NotFound x [4]; };
//	struct NonStaticData { NotFound x [5]; };
//
//	enum { NOT_FOUND = 0,
//		STATIC_FUNCTION = sizeof( StaticFunction ) - sizeof( NotFound ),
//		NON_STATIC_FUNCTION = sizeof( NonStaticFunction ) - sizeof( NotFound ),
//		STATIC_DATA = sizeof( StaticData ) - sizeof( NotFound ),
//		NON_STATIC_DATA = sizeof( NonStaticData ) - sizeof( NotFound ) };
//
//}
//
//#define CREATE_FUNCTION_DETECTOR(Identifier) \
//	namespace Detector { \
//	template < class T, \
//class NonStaticType, \
//class NonStaticConstType, \
//class StaticType > \
//class DetectMember_##Identifier { \
//	template < NonStaticType > \
//struct TestNonStaticNonConst ; \
//	\
//	template < NonStaticConstType > \
//struct TestNonStaticConst ; \
//	\
//	template < StaticType > \
//struct TestStatic ; \
//	\
//	template <class U > \
//	static NonStaticFunction Test( TestNonStaticNonConst<&U::Identifier>*, ... ); \
//	\
//	template <class U > \
//	static NonStaticFunction Test( TestNonStaticConst<&U::Identifier>*, int ); \
//	\
//	template <class U> \
//	static StaticFunction Test( TestStatic<&U::Identifier>*, int ); \
//	\
//	template <class U> \
//	static NotFound Test( ... ); \
//public : \
//	enum { check = sizeof( Test<T>( 0, 0 ) ) - sizeof( NotFound ) }; \
//};\
//} 
//
//#define DETECT_FUNCTION(Class,ReturnType,Identifier,Params) \
//	Detector::DetectMember_##Identifier< Class,\
//	ReturnType (Class::*)Params,\
//	ReturnType (Class::*)Params const,\
//	ReturnType (*)Params \
//	>::check

#endif