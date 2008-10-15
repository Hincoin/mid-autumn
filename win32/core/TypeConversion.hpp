#ifndef MA_TYPECONVERSION_HPP
#define MA_TYPECONVERSION_HPP


#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/push_back.hpp>
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

#include <boost/mpl/next.hpp>

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
	struct details{
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
	typedef typename details::do_promote<T,details::is_top_promote<T>::value>::type types;
public:
	typedef types type;
};

template<typename Sequence>
struct do_promotions{
	template<typename Start_Iter,typename End_iter,typename Set>
	struct seq2set;
	template<typename End_iter,typename Set>
	struct seq2set<End_iter,End_iter,Set>{
		typedef Set type;
	};
	template<typename Start_iter,typename End_iter,typename Set>
	struct seq2set{
		typedef typename boost::mpl::next<Start_iter>::type next_iter;
		typedef typename boost::mpl::insert<Set,typename boost::mpl::deref<Start_iter>::type>::type tmp_result;
		typedef typename seq2set<next_iter,End_iter,tmp_result>::type type;
	};

	//for every element in sequence
	typedef boost::mpl::_1 _1;
	typedef typename boost::mpl::copy<Sequence,boost::mpl::back_inserter<boost::mpl::vector<> > >::type vector_types;
	typedef typename boost::mpl::transform<vector_types,promote_types<_1> >::type vector_results;//sequence of sequence
	typedef typename seq2set<typename boost::mpl::begin<vector_results>::type,
		typename boost::mpl::end<vector_results>::type,
		boost::mpl::set<> >::type type;
	//typedef vector_results type;
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

template<typename BeginIter,typename EndIter,typename ResultSet,typename TypeStack>
struct dps_promote;



template<typename EndIter,typename ResultSet,typename TypeStack>
struct dps_promote<EndIter,EndIter,ResultSet,TypeStack>{
	typedef boost::mpl::set<> type;
};
template<typename StartIter,typename EndIter,typename ResultSet,typename TypeStack>
struct dps_promote
{
	//typedef typename multiple_promote<typename boost::mpl::deref<StartIter>::type>::type first_type_promote_set;
	//typedef 
	//typedef dps_promote<
	//typedef 
};

template<typename T>
struct multiple_promote;


template<typename T>
struct multiple_promote{
	typedef typename boost::mpl::vector<T> test_set ;
	typedef typename dps_promote<
		typename boost::mpl::begin<test_set>::type,
		typename boost::mpl::end<test_set>::type,
		boost::mpl::set<> ,
		test_set >::type type;
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




#endif
