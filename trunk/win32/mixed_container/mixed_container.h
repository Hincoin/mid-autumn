#include <boost/tuple/tuple.hpp>
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
#include <boost/type_traits.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <typeinfo>
#include "common.h"

//crtp
template<typename Derived>
class Shape{
	typedef Shape<Derived> class_type;
	typedef Derived impl_type;
	impl_type& derived(){return static_cast<impl_type&>(*this);}
	const impl_type& derived()const{return static_cast<const impl_type&>(*this);}
public:
	void draw(){return derived().drawImpl();}
	bool intersect(int seed){return derived().intersectImpl(seed) ;}
};

class simple_shape:public Shape<simple_shape>
{
	friend class Shape<simple_shape>;
	vector<void*> stored;
	void drawImpl(){/*printf("draw simple_shape\n");*/draw_function(stored);};
	bool intersectImpl(int seed){/*printf("intersect simple_shape\n");*/return intersect_function(stored,seed) ;}
public:
	~simple_shape(){destruct(stored);}
};

class simple_shape2:public Shape<simple_shape2>
{
	friend class Shape<simple_shape2>;
	vector<void*> stored;
	void drawImpl(){/*printf("draw simple_shape2 \n");*/
		draw_function(stored);};
	bool intersectImpl(int seed){/*printf("intersect simple_shape2 \n");*/
		return intersect_function(stored,seed);}
public:
	~simple_shape2(){destruct(stored);}
};


class simple_shape3:public Shape<simple_shape3>
{
	friend class Shape<simple_shape3>;
	vector<void*> stored;
	void drawImpl(){/*printf("draw simple_shape2 \n");*/
		draw_function(stored);};
		bool intersectImpl(int seed){/*printf("intersect simple_shape2 \n");*/
			return intersect_function(stored,seed);}
public:
	~simple_shape3(){destruct(stored);}
};

class simple_shape4:public Shape<simple_shape4>
{
	friend class Shape<simple_shape4>;
	vector<void*> stored;
	void drawImpl(){/*printf("draw simple_shape2 \n");*/
		draw_function(stored);};
		bool intersectImpl(int seed){/*printf("intersect simple_shape2 \n");*/
			return intersect_function(stored,seed);}
public:
	~simple_shape4(){destruct(stored);}
};

class simple_shape5:public Shape<simple_shape5>
{
	friend class Shape<simple_shape5>;
	vector<void*> stored;
	void drawImpl(){/*printf("draw simple_shape2 \n");*/
		draw_function(stored);};
		bool intersectImpl(int seed){/*printf("intersect simple_shape2 \n");*/
			return intersect_function(stored,seed);}
public:
	~simple_shape5(){destruct(stored);}
};

class simple_shape6:public Shape<simple_shape6>
{
	friend class Shape<simple_shape6>;
	vector<void*> stored;
	void drawImpl(){/*printf("draw simple_shape2 \n");*/
		draw_function(stored);};
		bool intersectImpl(int seed){/*printf("intersect simple_shape2 \n");*/
			return intersect_function(stored,seed);}
public:
	~simple_shape6(){destruct(stored);}
};

class simple_shape7:public Shape<simple_shape7>
{
	friend class Shape<simple_shape7>;
	vector<void*> stored;
	void drawImpl(){/*printf("draw simple_shape2 \n");*/
		draw_function(stored);};
		bool intersectImpl(int seed){/*printf("intersect simple_shape2 \n");*/
			return intersect_function(stored,seed);}
public:
	~simple_shape7(){destruct(stored);}
};

class simple_shape8:public Shape<simple_shape8>
{
	friend class Shape<simple_shape8>;
	vector<void*> stored;
	void drawImpl(){/*printf("draw simple_shape2 \n");*/
		draw_function(stored);};
		bool intersectImpl(int seed){/*printf("intersect simple_shape2 \n");*/
			return intersect_function(stored,seed);}
public:
	~simple_shape8(){destruct(stored);}
};

class simple_shape9:public Shape<simple_shape9>
{
	friend class Shape<simple_shape9>;
	vector<void*> stored;
	void drawImpl(){/*printf("draw simple_shape2 \n");*/
		draw_function(stored);};
		bool intersectImpl(int seed){/*printf("intersect simple_shape2 \n");*/
			return intersect_function(stored,seed);}
public:
	~simple_shape9(){destruct(stored);}
};

class simple_shape0:public Shape<simple_shape0>
{
	friend class Shape<simple_shape0>;
	vector<void*> stored;
	void drawImpl(){/*printf("draw simple_shape2 \n");*/
		draw_function(stored);};
		bool intersectImpl(int seed){/*printf("intersect simple_shape2 \n");*/
			return intersect_function(stored,seed);}
public:
	~simple_shape0(){destruct(stored);}
};
namespace mpl = boost::mpl;
template< typename Tuple > struct tuple_iterator
{
	typedef mpl::forward_iterator_tag category;
	typedef typename Tuple::head_type type;
	typedef tuple_iterator<typename Tuple::tail_type> next;
};

// end iterator
template<> struct tuple_iterator<boost::tuples::null_type>
{
	typedef mpl::forward_iterator_tag category;
};

template< typename Tuple > struct tuple_sequence
: mpl::iterator_range<
tuple_iterator<Tuple>
, tuple_iterator<boost::tuples::null_type>
>
{
}; 
//////////////////////////////////////////////////////////////////////////
//construct tuples from mpl vector
//this can be 
//RegistShapeBegin
//RegistNewShape()
//RegistShapeEnd //do a compile time check if there is duplicated type

//typedef boost::tuples::tuple<
//std::vector<simple_shape>,
//std::vector<simple_shape2> 
//> ShapeVector;
//
//typedef tuple_sequence<ShapeVector> shape_array_sequence_t;
//
//template<typename T>
//struct extract_type{
//	typedef typename T::value_type type;
//};
//
//typedef boost::mpl::transform<boost::mpl::list<std::vector<simple_shape>,std::vector<simple_shape2> >,extract_type<boost::mpl::_1> >::type shape_type_array;

#include <boost/tuple/tuple.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/copy.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp> 
#define _BOOST_MAX_TUPLE_TYPE_ 10
template<class T, class Tuple>
struct tuple_push_front;

template<class T, BOOST_PP_ENUM_PARAMS(_BOOST_MAX_TUPLE_TYPE_, class T)>
struct tuple_push_front<T, boost::tuple<BOOST_PP_ENUM_PARAMS(_BOOST_MAX_TUPLE_TYPE_, T)> > {
	typedef boost::tuple<T, BOOST_PP_ENUM_PARAMS( BOOST_PP_DEC(_BOOST_MAX_TUPLE_TYPE_), T)> type;
};
#undef _BOOST_MAX_TUPLE_TYPE_
template<class Sequence>
struct to_tuple {
	typedef typename boost::mpl::reverse_copy<
		Sequence, boost::mpl::inserter<boost::tuple<>,
		tuple_push_front<boost::mpl::_2, boost::mpl::_1> >
	>::type type;
}; 


typedef boost::mpl::list<
simple_shape,
simple_shape2,
simple_shape3,
simple_shape4,
simple_shape5,
simple_shape6,
simple_shape7,
simple_shape8,
simple_shape9,
simple_shape0
> shape_type_list;

template<typename T>
struct add_container{
	typedef std::vector<T> type;
};

template<typename T>
struct add_shared_ptr{
	typedef boost::shared_ptr<T> type;
};
typedef boost::mpl::transform<shape_type_list,add_container<boost::mpl::_1> >::type shape_array_sequence_t;
typedef boost::mpl::transform<shape_type_list,boost::add_pointer<boost::mpl::_1> >::type shape_ptr_type_list;
typedef boost::mpl::transform<shape_type_list,add_shared_ptr<boost::mpl::_1> >::type shared_shape_ptr_type_list;
typedef to_tuple<shape_array_sequence_t>::type ShapeVector;

template<typename VectorTuple = ShapeVector>
class MixedShapeVector:public Shape<MixedShapeVector< VectorTuple > >{
	friend class Shape<MixedShapeVector<VectorTuple > >;
	//typedef tuple_sequence<VectorTuple> shape_array_sequence;
	typedef shape_array_sequence_t shape_array_sequence;
	VectorTuple impl_;
private:
	template<int SZ>
	struct for_draw;
	template<> struct for_draw<0>{
		static void exec(VectorTuple& impl)
		{
			typedef typename boost::mpl::at_c<shape_array_sequence,0>::type vector_type;
			vector_type& v = boost::tuples::get<0>(impl);
			//printf("%s\n",typeid(vector_type).name());
			vector_type::iterator the_end = v.end();
			for(vector_type::iterator it  = v.begin(); it != the_end; ++it)
				(*it).draw();
		};
	};
	template<int SZ> struct for_draw{
		static void exec(VectorTuple& impl)
		{
			typedef tuple_sequence<VectorTuple> shape_array_sequence;
			typedef typename boost::mpl::at_c<shape_array_sequence, SZ>::type vector_type;
			vector_type& v = boost::tuples::get<SZ>(impl);
			//printf("%s\n",typeid(vector_type).name());
			typename vector_type::iterator the_end = v.end();
			for(typename vector_type::iterator it  = v.begin(); it != the_end; ++it)
				(*it).draw();
			return for_draw<SZ-1>::exec(impl);
		}
	};

	template<int SZ> struct for_intersect;
	template<> struct for_intersect<0>{
		static bool exec(VectorTuple& impl,int seed)
		{
			bool ret = false;
			typedef typename boost::mpl::at_c<shape_array_sequence,0>::type vector_type;
			vector_type& v = boost::tuples::get<0>(impl);
			vector_type::iterator the_end = v.end();
			for(vector_type::iterator it  = v.begin(); it != the_end; ++it)
				ret = ret ||  (*it).intersect(seed);
			return ret;
		};
	};
	template<int SZ> struct for_intersect{
		static bool exec(VectorTuple& impl,int seed)
		{
			bool ret = false;
			typedef typename boost::mpl::at_c<shape_array_sequence,SZ>::type vector_type;
			vector_type& v = boost::tuples::get<SZ>(impl);
			vector_type::iterator the_end = v.end();
			for(vector_type::iterator it  = v.begin(); it != the_end; ++it)
				ret = ret || (*it).intersect(seed);
			return ret = ret || for_intersect<SZ-1>::exec(impl,seed);
		}
	};
	template<typename _T,int N>
	struct is_the_type{
		static const bool value = 
			boost::is_same<_T,typename boost::mpl::at_c<shape_array_sequence,N>::type >::value;
	};
	template<typename _T,int N,bool> struct get_idx_by_type;
	template<typename _T,int N>
	struct get_idx_by_type<_T,N,true>{enum{value = N};};
	template<typename _T,int N>
	struct get_idx_by_type<_T,N,false>{
		//typedef tuple_sequence<VectorTuple> shape_array_sequence;
		enum{value = get_idx_by_type<_T,N+1,is_the_type<_T,N+1>::value>::value};
	};

	template<typename _T>
	struct get_type_idx{
		enum{value = get_idx_by_type<_T,0,is_the_type<_T,0>::value >::value};
	};

private:
	//interface implementation
	//foreach element do the functions
	void drawImpl(){
		for_draw<boost::mpl::size<shape_array_sequence>::value - 1>::exec(impl_);
	}
	bool intersectImpl(int seed){
		return for_intersect<boost::mpl::size<shape_array_sequence>::value - 1>::exec(impl_,seed);
	}
public:
	template<typename SP>
	void push_back(const SP& v){
		//printf("get type idx %d",get_type_idx< std::vector<SP> >::value);
		boost::tuples::get<get_type_idx< std::vector<SP> >::value>(impl_).push_back(v);
	}
	
};

//the other way to using boost.variant


#include <boost/variant.hpp>
#include <algorithm>

typedef boost::make_variant_over<shape_type_list>::type generic_shape;
typedef boost::make_variant_over<shape_ptr_type_list>::type generic_shape_ptr;
typedef boost::make_variant_over<shared_shape_ptr_type_list>::type generic_shared_shape_ptr;

template<typename T>
struct is_shared_ptr;
template<typename T>
struct is_shared_ptr<boost::shared_ptr<T> >:boost::true_type{};

template<typename T>
struct is_shared_ptr:boost::false_type{};

BOOST_STATIC_ASSERT((is_shared_ptr<boost::shared_ptr<int> >::value));
namespace details{
	struct draw_visitor: public boost::static_visitor<>
	{
		typedef void result_type;
	private:
		template<typename S>
		result_type impl(S& s,boost::true_type&)const{
			return s->draw();
		}
		template<typename S>
		result_type impl(S& s,boost::false_type&)const{
			return s.draw();
		}

	public:
		
		template<typename S>
		void operator()(S& s)const{
			//printf("%s\n",typeid(S).name());
			return impl(s,
				boost::integral_constant<
				bool,
				boost::is_pointer<S>::value || 
				is_shared_ptr<S>::value
				>::type());
		}
	};
	struct intersect_visitor:public boost::static_visitor<>
	{
		typedef bool result_type;
	private:
		int seed;
		template<typename S>
		result_type impl(S& s, boost::true_type&)const
		{
			return s->intersect(seed);
		}
		template<typename S>
		result_type impl(S& s, boost::false_type&)const
		{
			return s.intersect(seed);
		}
	public:
		intersect_visitor(int s):seed(s){}
		template<typename S>
		bool operator()(S& s)const
		{
			return impl(s,
				boost::integral_constant<
				bool,
				boost::is_pointer<S>::value || 
				is_shared_ptr<S>::value
				>());
		}
	};
	struct destroy_visitor:public boost::static_visitor<>{
		typedef void result_type;
		template<typename S>
		result_type impl(S& s, boost::false_type&)const{}
		template<typename S>
		result_type impl(S& s, boost::true_type&)const{
			delete s;
		}
	public:
		template<typename S>
		result_type operator()(S& s)const
		{
			impl(s,boost::is_pointer<S>());
		};
	};
}
template<typename V>
struct default_draw{
	typedef void draw_result_type;
	static draw_result_type draw_(vector<V>& v){
		std::for_each(v.begin(),v.end(),boost::apply_visitor(details::draw_visitor()));
	}
};
template <typename V>
struct default_intersect{
	typedef bool intersect_result_type;
	static intersect_result_type intersect_(vector<V>& v,int seed)
	{
		typedef vector<V>::iterator v_iterator;
		boost::apply_visitor_delayed_t<details::intersect_visitor>
			elem_visitor = boost::apply_visitor(details::intersect_visitor(seed));
		for (v_iterator it = v.begin(); it != v.end(); ++it)
		{
			if (elem_visitor(*it))
			{
				return true;
			}
		}
		return false;
	};
};

template <typename V>
struct default_destroy{
	typedef void destroy_result_type;
	static destroy_result_type destroy_(vector<V>& v)
	{
		std::for_each(v.begin(),v.end(),boost::apply_visitor(details::destroy_visitor()));
	};
};
namespace details{
	struct empty_type{};
	template<typename SeqIter,typename EndIter>
	struct generate_hiearachy_impl;
	template<typename EndIter>
	struct generate_hiearachy_impl<EndIter,EndIter>{};
	template<typename SeqIter,typename EndIter>
	struct generate_hiearachy_impl:
		public boost::mpl::deref<SeqIter>::type,
		public generate_hiearachy_impl<typename boost::mpl::next<SeqIter>::type,EndIter>
	{};

}

//generate linear hiearachy over type sequence
template<typename Seq>
struct generate_hiearachy:public details::generate_hiearachy_impl<typename boost::mpl::begin<Seq>::type,typename boost::mpl::end<Seq>::type>
{};

template<typename V = generic_shape,class PolicySeq = boost::mpl::list<default_draw<V>,default_intersect<V>,default_destroy<V> >  >
class ShapeVariantVector:
	public generate_hiearachy<PolicySeq>,
	public Shape<ShapeVariantVector<V,PolicySeq> >
{
	typedef generate_hiearachy<PolicySeq> policy_type;
	friend class Shape<ShapeVariantVector<V,PolicySeq> >;
	typedef V value_type;
	std::vector<value_type> impl;
private:
	typename policy_type::draw_result_type drawImpl(){return policy_type::draw_(impl);}
	typename policy_type::intersect_result_type intersectImpl(int seed){return policy_type::intersect_(impl,seed);}
public:
	void push_back(const value_type& v){impl.push_back(v);}
	~ShapeVariantVector(){policy_type::destroy_(impl);};
};
