
///////////////////////////////////////////////////////////////////////////////
// Copyright 2007 Alexandre Courpron
//
// Permission to use, copy, modify, redistribute and sell this software, 
// provided that this copyright notice appears on all copies of the software.
///////////////////////////////////////////////////////////////////////////////

#ifndef HASMEMBER_HPP
#define HASMEMBER_HPP





#define CREATE_MEMFUN_DETECTOR(Identifier) \
	namespace FunctionDetector {\
typedef char NotFound;\
struct StaticFunction { NotFound x [2]; };\
struct NonStaticFunction { NotFound x [3]; };\
struct StaticData { NotFound x [4]; };\
struct NonStaticData { NotFound x [5]; };\
\
enum { NOT_FOUND = 0,\
STATIC_FUNCTION = sizeof( StaticFunction ) - sizeof( NotFound ),\
NON_STATIC_FUNCTION = sizeof( NonStaticFunction ) - sizeof( NotFound ),\
STATIC_DATA = sizeof( StaticData ) - sizeof( NotFound ),\
NON_STATIC_DATA = sizeof( NonStaticData ) - sizeof( NotFound ) };\
\
}\
	namespace FunctionDetector { \
	template < class T, \
class NonStaticType, \
class NonStaticConstType, \
class StaticType > \
class DetectMember_##Identifier { \
	template < NonStaticType > \
struct TestNonStaticNonConst ; \
	\
	template < NonStaticConstType > \
struct TestNonStaticConst ; \
	\
	template < StaticType > \
struct TestStatic ; \
	\
	template <class U > \
	static NonStaticFunction Test( TestNonStaticNonConst<&U::Identifier>*, ... ); \
	\
	template <class U > \
	static NonStaticFunction Test( TestNonStaticConst<&U::Identifier>*, int ); \
	\
	template <class U> \
	static StaticFunction Test( TestStatic<&U::Identifier>*, int ); \
	\
	template <class U> \
	static NotFound Test( ... ); \
public : \
	enum { value = sizeof( Test<T>( 0, 0 ) ) - sizeof( NotFound ) }; \
};\
} 

#define HAS_MEMBER_FUNCTION(Class,ReturnType,Identifier,Params) \
	FunctionDetector::DetectMember_##Identifier< Class,\
	ReturnType (Class::*)Params,\
	ReturnType (Class::*)Params const,\
	ReturnType (*)Params \
	>::value


//#ifndef _MSC_VER
//#define CREATE_DATA_DETECTOR(Identifier) \
//	namespace Detector { \
//	template < class T, \
//class NonStaticType, \
//class StaticType > \
//class DetectMember_##Identifier { \
//	template < NonStaticType > \
//struct TestNonStatic ; \
//	\
//	template < StaticType > \
//struct TestStatic ; \
//	\
//	template <class U > \
//	static NonStaticData Test( TestNonStatic<&U::Identifier>* ); \
//	\
//	template <class U> \
//	static StaticData Test( TestStatic<&U::Identifier>* ); \
//	\
//	template <class U> \
//	static NotFound Test( ... ); \
//public : \
//	enum { check = sizeof( Test<T>( 0 ) ) - sizeof( NotFound ) }; \
//};\
//} 
//
//#define DETECT_DATA(Class,Type,Identifier) \
//	Detector::DetectMember_##Identifier< Class,\
//	Type (Class::*),\
//	Type (*)\
//	>::check
//#endif



#endif