#ifndef MA_TYPECONVERSION_HPP
#define MA_TYPECONVERSION_HPP


#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/variant.hpp>

#include <boost/static_assert.hpp>

#include <boost/mpl/max_element.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/sizeof.hpp>

#include <boost/mpl/set.hpp>
#include <boost/mpl/copy.hpp>
#include <boost/mpl/insert.hpp>

#include <boost/mpl/transform.hpp>
#include <boost/mpl/remove_if.hpp>
#include <boost/mpl/next.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/size.hpp>

//meta-function for type promotion

//no dead-loop promotion allowed
//such as a promote to b , b promote to c, c promote to a

//multiple promotion need to be done


template<typename T>
struct type_promote{
	//typedef boost::mpl::vector<T> type;
	typedef T type;
};

//if has loop it will generate a compile error
//else the type will be an linear chain of type promotion
template<typename T>
struct type_promote_loop_checker{
private:
	template<typename U,typename Seq,bool>
	struct impl;
	template<typename U,typename Seq>
	struct impl<U,Seq,true>{
		typedef Seq type;
	};
	template<typename U,typename Seq>
	struct impl<U,Seq,false>{
		enum{value = boost::is_same<typename boost::mpl::find<Seq,U>::type,typename boost::mpl::end<Seq>::type>::value};
		BOOST_STATIC_ASSERT(value == 0);
		typedef typename boost::mpl::push_back<Seq,U>::type pushed_type;
		typedef typename type_promote<U>::type cur_promoted;
		typedef typename impl<cur_promoted,Seq,boost::is_same<cur_promoted,U>::value>::type type;
	};
public:
	typedef typename impl<T,boost::mpl::vector<> , boost::is_same<T,typename type_promote<T>::type>::value>::type type;
};
template<typename T>
struct promote_types{
private:
	struct _details_{
		template<typename U>
		struct is_top_promote:boost::is_same<U,typename type_promote<U>::type>
		{};

		template<typename U,bool>
		struct do_promote;

		template<typename U>
		struct do_promote<U,true >{
			typedef boost::mpl::set<U> type;
		};

		template<typename U>
		struct do_promote<U,false>
		{
			//dead loop check to see if U is already in the mpl::vector

			typedef typename boost::mpl::insert<
				typename do_promote<
				typename type_promote<U>::type ,is_top_promote<U>::value
				>::type,U
			>::type type;

		};

	};
	typedef typename _details_::template do_promote<T,_details_::template is_top_promote<T>::value>::type types;
public:
	typedef types type;
};

template<typename Sequence>
struct do_promotions{
private:
	//for every element in sequence
	typedef boost::mpl::_1 _1;
	typedef typename boost::mpl::copy<Sequence,boost::mpl::back_inserter<boost::mpl::vector<> > >::type vector_types;
	typedef typename boost::mpl::transform<vector_types,promote_types<_1> >::type vector_results;//sequence of sequence
public:
	typedef typename boost::mpl::fold<vector_results,boost::mpl::set<>,boost::mpl::insert<boost::mpl::placeholders::_1,boost::mpl::placeholders::_2> >::type type;

};

//it's ugly now can be simply beautified
template<typename SeqOfSeq,typename result_seq>
struct flatten_seq_seq_impl{
private:
	typedef boost::mpl::_1 _1;
	typedef boost::mpl::_2 _2;

	template<typename S,typename R,typename CurIterator,bool>
	struct iterate_insert_S;

	template<typename S,typename R,typename CurIter>
	struct iterate_insert_S<S,R,CurIter,true>{
		typedef R type;
	};
	template<typename S,typename R,typename CurIter>
	struct iterate_insert_S<S,R,CurIter,false>
	{
		typedef typename boost::mpl::deref<CurIter>::type cur_type;
		typedef typename boost::mpl::next<CurIter>::type next_iter;
		typedef typename boost::mpl::insert<R,cur_type>::type cur_result;
		typedef typename iterate_insert_S<S,cur_result,next_iter,boost::is_same<next_iter,typename boost::mpl::end<S>::type>::value >::type type;
	};

	template<typename SOS,typename ResultSet,typename CurIterator,bool>
	struct iterate_insert;
	template<typename SOS,typename ResultSet,typename CurIterator>
	struct iterate_insert<SOS,ResultSet,CurIterator,true>{
		typedef ResultSet type;
	};
	template<typename SOS,typename ResultSet,typename CurIterator>
	struct iterate_insert<SOS,ResultSet,CurIterator,false>
	{
		typedef typename boost::mpl::deref<CurIterator>::type cur_set;
		typedef typename boost::mpl::begin<cur_set>::type cur_begin;
		typedef typename iterate_insert_S<cur_set,ResultSet,cur_begin,boost::is_same<cur_begin,typename boost::mpl::end<cur_set>::type>::value>::type cur_result;

		typedef typename boost::mpl::next<CurIterator>::type next_iter;

		typedef typename iterate_insert<SOS,cur_result,next_iter,
			boost::is_same<next_iter,typename boost::mpl::end<SOS>::type>::value >::type type;
	};
	typedef typename boost::mpl::begin<SeqOfSeq>::type begin_iterator;
public:
	typedef typename iterate_insert<SeqOfSeq,result_seq,begin_iterator,boost::is_same<begin_iterator,typename boost::mpl::end<SeqOfSeq>::type>::value>::type type;
};
template<typename SeqOfSeq>
struct flatten_seq_seq{
	typedef boost::mpl::set<> result_type;
	typedef typename flatten_seq_seq_impl<SeqOfSeq,result_type>::type type;
};

template<typename T>
struct recursive_type_promote{
private:
	template<typename CurSeq>
	struct apply_do_promotions{
		typedef typename do_promotions<CurSeq>::type type;
	};
	template<typename SeqOfSeq,bool>
	struct apply_flatten;
	template<typename SeqOfSeq>
	struct apply_flatten<SeqOfSeq,true>
	{
		typedef typename flatten_seq_seq<SeqOfSeq>::type type;
	};
	template<typename SeqOfSeq>
	struct apply_flatten<SeqOfSeq,false>{
		typedef typename flatten_seq_seq<SeqOfSeq>::type cur_seq_flattened;//
		typedef typename apply_do_promotions<cur_seq_flattened>::type cur_promoted_types;//sequence of sequence
		typedef typename apply_flatten<cur_promoted_types,boost::is_same<cur_promoted_types,SeqOfSeq>::value>::type type;
	};
	//init states
	typedef typename promote_types<T>::type cur_seq_flattened;//
	typedef typename apply_do_promotions<cur_seq_flattened>::type cur_promoted_types;//sequence of sequence
	typedef typename flatten_seq_seq<cur_promoted_types>::type next_seq_flattened;
public:
	typedef typename apply_flatten<cur_promoted_types,boost::is_same<cur_seq_flattened,next_seq_flattened>::value>::type type;
};

template<typename Seq0,typename Seq1>
struct same_type_seq{
	template<typename Seq_Iter,typename Seq_End,typename Seq_1>
	struct impl;

	template<typename Seq_End,typename Seq_1>
	struct impl<Seq_End,Seq_End,Seq_1>{
		enum{value = true};
	};

	template<typename Seq_Iter,typename Seq_End,typename Seq_1>
	struct impl{
		typedef typename boost::mpl::find<Seq_1,typename boost::mpl::deref<Seq_Iter>::type>::type find_iter;
		enum{value =
			! boost::is_same<find_iter,typename boost::mpl::end<Seq_1>::type>::value
			&& impl<typename boost::mpl::next<Seq_Iter>::type,Seq_End,Seq_1>::value};
	};
	enum{value = boost::mpl::size<Seq0>::value == boost::mpl::size<Seq1>::value &&
		impl<typename boost::mpl::begin<Seq0>::type,typename boost::mpl::end<Seq0>::type,Seq1>::value  &&
		impl<typename boost::mpl::begin<Seq1>::type,typename boost::mpl::end<Seq1>::type,Seq0>::value
	};
};

template<typename T>
struct multiple_type_promote{
	typedef boost::mpl::vector<T> type;
};


template<typename OldStack,typename TypeStack,typename ResultSet>
struct dfs_promote;

template<typename TypeStack,typename ResultSet>
struct dfs_promote<TypeStack,TypeStack,ResultSet>{
	typedef ResultSet type;
};
template<typename OldStack,typename TypeStack,typename ResultSet>
struct dfs_promote
{
private:
	typedef boost::mpl::_ _;

	typedef typename boost::mpl::front<TypeStack>::type current_top;
	typedef typename multiple_type_promote<current_top>::type promote_types;

	BOOST_STATIC_ASSERT((!boost::mpl::empty<TypeStack>::value));
	typedef typename boost::mpl::pop_front<TypeStack>::type popped_stack;
	//make new_stack
	typedef typename boost::mpl::copy<promote_types,
		boost::mpl::back_inserter<popped_stack> >::type new_dup_seq;

	typedef typename boost::mpl::remove_if<new_dup_seq,boost::is_same<_,current_top> >::type new_seq;
	//make type unique in the stack
	typedef typename boost::mpl::fold<new_seq,boost::mpl::set<>,boost::mpl::insert<boost::mpl::placeholders::_1,boost::mpl::placeholders::_2> >::type new_set_for_stack;
	typedef typename boost::mpl::copy<new_set_for_stack,boost::mpl::back_inserter<boost::mpl::vector<> > >::type new_stack;

	typedef typename boost::mpl::insert<ResultSet,current_top>::type new_result;

	typedef typename boost::mpl::if_c<same_type_seq<TypeStack,new_stack>::value,new_stack,TypeStack>::type old_stack;
	typedef typename boost::mpl::if_<typename boost::mpl::empty<new_stack>::type,old_stack,new_stack>::type passed_new_stack;
public:

	typedef typename dfs_promote<old_stack,passed_new_stack,new_result>::type type;
};

template<typename T>
struct multiple_promote;


template<typename T>
struct multiple_promote{
private:
	typedef typename multiple_type_promote<T>::type test_set ;
	typedef typename dfs_promote<
		boost::mpl::vector<T>,
		test_set,
		boost::mpl::set<T> >::type set_type;
public:
	typedef typename boost::mpl::copy<set_type,boost::mpl::back_inserter<boost::mpl::vector<> > >::type type;
};


template<typename T>
struct promote_to_variant{
public:
	typedef typename boost::make_variant_over< typename promote_types<T>::type >::type type;
};

template<typename Sequence>
struct biggest_type
{
private:
	typedef boost::mpl::_1 _1;
	typedef typename boost::mpl::max_element<
		boost::mpl::transform_view<Sequence,boost::mpl::sizeof_<_1> >
	>::type result_iter;
public:
	typedef typename boost::mpl::deref<typename result_iter::base>::type type;
};

#include <boost/mpl/at.hpp>
#include <boost/mpl/int.hpp>
#include "OrderedTypeInfo.hpp"
#include "AssocVector.hpp"

#include <vector>
#include <algorithm>

#ifdef _DEBUG
#include <iostream>
#endif
template<typename T>
struct cast_types{
	typedef typename multiple_promote<T>::type type;
	typedef typename boost::mpl::if_c<(sizeof(typename biggest_type<type>::type) <= sizeof(T)),
		T,
		typename biggest_type<type>::type>::type stored_type;

	BOOST_STATIC_ASSERT(( boost::mpl::size<type>::value >0 ));
	template<typename TO_U>
	static const void* static_cast_func_impl(const stored_type* const x,const boost::true_type&){
		BOOST_STATIC_ASSERT((boost::is_convertible<stored_type,TO_U&>::value));
		return (void*)& (static_cast<const TO_U&>(*x));
	} 
	template<typename TO_U>
	static const void* static_cast_func_impl(const stored_type* const x,const boost::false_type&){
		return x;
	} 
private:
	typedef const void* (*SafeCastFunc)(const stored_type* const);

	template<typename TO_U>
	static const void* static_cast_func(const stored_type* const x){
		return static_cast_func_impl<TO_U>(x,boost::is_convertible<stored_type,TO_U&>() );
	}

	typedef AssocVector<TypeInfo,SafeCastFunc> TypeCastFuncMap;
	template<typename Seq,size_t N>
	struct init_type_info_impl;

	template<typename Seq>
	struct init_type_info_impl<Seq,0>{
		void operator()(TypeCastFuncMap& result)
		{
			typedef typename boost::mpl::at<Seq,boost::mpl::int_<0> >::type to_type;
			result.insert(std::make_pair(TypeInfo(typeid(to_type)),&static_cast_func<to_type>));
		}
	};

	template<typename Seq,size_t N>
	struct init_type_info_impl{
		void operator()(TypeCastFuncMap& result)
		{
			init_type_info_impl<Seq,N-1> n_1_type;
			n_1_type(result);
			typedef typename boost::mpl::at<Seq,boost::mpl::int_<N> >::type to_type;
			result.insert(std::make_pair(TypeInfo(typeid(to_type)),&static_cast_func<to_type>));
		}
	};
	static TypeCastFuncMap init_type_info()
	{
		TypeCastFuncMap ret;
		init_type_info_impl<type,boost::mpl::size<type>::value -1 > impl;
		impl(ret);
		return ret;
	}


public:
	static const TypeCastFuncMap casted_types_info;
	static const void* cast_to(const TypeInfo& t_info,const void* const x){
		typename TypeCastFuncMap::const_iterator it(casted_types_info.find(t_info));
		return it == casted_types_info.end()? 0: (*(it->second))(reinterpret_cast<const stored_type* const>(x));
	}
	template<typename TO_U>
	static typename boost::mpl::if_<boost::is_convertible<stored_type,TO_U&>,const TO_U&,TO_U>::type
	cast_to(const void* const x)
	{
		typedef typename boost::mpl::if_<boost::is_convertible<stored_type,TO_U&>,const TO_U&,TO_U>::type result_type;
		return static_cast<result_type>(*(stored_type*)x);
		//SafeCastFunc fun_ptr = &static_cast_func<TO_U>;
		//const void * r = (*fun_ptr)((stored_type*)x);
		//return *(const TO_U*)(r);
	}
};
template<>
struct cast_types<void>
{
	typedef void* stored_type;
	static const void* cast_to(const TypeInfo& t_info,const void* const x){
		return typeid(void*) == t_info? 0: x;
	}
};
template<typename T>
const typename cast_types<T>::TypeCastFuncMap cast_types<T>::casted_types_info = cast_types<T>::init_type_info();



template<>
struct multiple_type_promote<char>
{
	typedef boost::mpl::vector<char,short,unsigned short,int,unsigned int,long,unsigned long,float,double> type;
};
template<>
struct multiple_type_promote<short>
{
	typedef boost::mpl::vector<short,int,unsigned int,long,unsigned long ,float,double> type;
};
template<>
struct multiple_type_promote<int>
{
	typedef boost::mpl::vector<int,long,unsigned long ,double> type;
};
template<>
struct multiple_type_promote<unsigned char>
{
	typedef boost::mpl::vector<unsigned char,short,int,unsigned short,unsigned int,long,float,double> type;
};
template<>
struct multiple_type_promote<unsigned short>
{
	typedef boost::mpl::vector<unsigned short,int,unsigned short,unsigned int,long,float,double> type;
};
template<>
struct multiple_type_promote<unsigned int>
{
	typedef boost::mpl::vector<unsigned int,long,double> type;
};

template<>
struct multiple_type_promote<long>
{
	typedef boost::mpl::vector<double> type;
};
template<>
struct multiple_type_promote<unsigned long>
{
	typedef boost::mpl::vector<double> type;
};

template<>
struct multiple_type_promote<float>
{
	typedef boost::mpl::vector<double> type;
};

#include <string>
template<>
struct multiple_type_promote<const char*>
{
	typedef boost::mpl::vector<std::string> type;
};

template<>
struct multiple_type_promote<char*>
{
	typedef boost::mpl::vector<std::string> type;
};
#endif
