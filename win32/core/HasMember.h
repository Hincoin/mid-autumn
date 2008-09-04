#ifndef HASMEMBER_H
#define HASMEMBER_H

template<typename T>
struct has_member_lock
{
	typedef char true_type;
	struct false_type
	{
		true_type dummy[2];
	};

	template<typename U>
	static true_type has_member(U*,void (U::*dummy)()=&U::lock);
	static false_type has_member(void*);

	BOOST_STATIC_CONSTANT(bool, value=sizeof(has_member_lock<T>::has_member((T*)NULL))==sizeof(true_type));
};


namespace Detector {
	typedef char NotFound;
	struct StaticFunction { NotFound x [2]; };
	struct NonStaticFunction { NotFound x [3]; };
	struct StaticData { NotFound x [4]; };
	struct NonStaticData { NotFound x [5]; };

	enum { NOT_FOUND = 0,
		STATIC_FUNCTION = sizeof( StaticFunction ) - sizeof( NotFound ),
		NON_STATIC_FUNCTION = sizeof( NonStaticFunction ) - sizeof( NotFound ),
		STATIC_DATA = sizeof( StaticData ) - sizeof( NotFound ),
		NON_STATIC_DATA = sizeof( NonStaticData ) - sizeof( NotFound ) };

}

#define CREATE_FUNCTION_DETECTOR(Identifier) \
	namespace Detector { \
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
	enum { check = sizeof( Test<T>( 0, 0 ) ) - sizeof( NotFound ) }; \
};\
} 

#define DETECT_FUNCTION(Class,ReturnType,Identifier,Params) \
	Detector::DetectMember_##Identifier< Class,\
	ReturnType (Class::*)Params,\
	ReturnType (Class::*)Params const,\
	ReturnType (*)Params \
	>::check

#endif