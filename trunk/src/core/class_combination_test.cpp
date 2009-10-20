#include <boost/mpl/transform.hpp>

#include "simple_test_framework.hpp"


bool class_combination_test();

REGISTER_TEST_FUNC(class_combination_test,class_combination_test)
#include <boost/fusion/container.hpp>
#include <boost/fusion/adapted/mpl.hpp>
template<template<typename MPL_Seq>class ToFusionSeq,typename Seq,template<typename Arg>class Fun>
struct combination1{
	typedef typename boost::mpl::transform<Seq,Fun<boost::mpl::_1> >::type mpl_type;
	typedef typename ToFusionSeq<mpl_type>::type type;
};
/*
template<typename T,typename Seq1,template<typename Arg1,typename Arg2>class Fun2>
struct combination2_helper{
	template<typename U>
	struct _helper:Fun2<T,U>{
		
	};
	typedef typename combination1<Seq1, _helper>::type type;

};
*/
template<template<typename MPL_Seq>class ToFusionSeq,typename Seq0,typename Seq1,
	template<typename Arg1,typename Arg2>class Fun2>
struct combination2{
	template<typename T>
	struct combination2_helper{
		template<typename U0>
			struct reduced:Fun2<T,U0>{};
		typedef typename combination1<ToFusionSeq,Seq1,reduced>::type type;
		//typedef typename combination1<Seq1,reduced>::fusion_seqence fusion_seqence;
	};
	typedef typename boost::mpl::transform<Seq0,combination2_helper<boost::mpl::_1> >::type mpl_type;
	//typedef combination1<Seq0	
	typedef typename ToFusionSeq<mpl_type>::type type;


};

template<template<typename MPL_Seq>class ToFusionSeq,typename Seq0,typename Seq1,typename Seq2,
	template<typename Arg1,typename Arg2,typename Arg3>class Fun3>
	struct combination3{
		template<typename T>
		struct combination3_helper{
			template<typename U0,typename U1>
			struct reduced:Fun3<T,U0,U1>{};
			typedef typename combination2<ToFusionSeq,Seq1,Seq2,reduced>::type type;	
		};	
		typedef typename boost::mpl::transform<Seq0,combination3_helper<boost::mpl::_1> >::type mpl_type;
		typedef typename ToFusionSeq<mpl_type>::type type;	
	};


#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/container/vector/vector_fwd.hpp>
#include <boost/fusion/include/vector_fwd.hpp>





















#include <stdio.h>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/list.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/equal.hpp>
template<typename T1,typename T2>
struct comp2{
	typedef comp2<T1,T2> type;

};
template<typename T1,typename T2,typename T3>
struct comp3{
	typedef comp3<T1,T2,T3> type;
	
	comp3(){
		printf("comp3<%s,%s,%s>\n",T1::get_type_str(),T2::get_type_str(),T3::get_type_str());
	}

};


struct A{
	static char* get_type_str(){return "A";}
};
struct B{
	static char* get_type_str(){return "B";}
};

struct C{
	static char* get_type_str(){return "C";}
};

struct D{
	static char* get_type_str(){return "D";}
};
struct E{
	static char* get_type_str(){return "E";}
};

struct F{
	static char* get_type_str(){return "F";}
};

bool class_combination_test()
{
	using namespace boost;
	typedef mpl::vector<A,B> seq_type1;
	typedef mpl::vector<C,D> seq_type2;
	typedef mpl::vector<E,F> seq_type3;

	typedef combination3<boost::fusion::result_of::as_vector,seq_type1,seq_type2,seq_type3,comp3>::type comb3_types;
    //    printf("combination3 string: %s \n",typeid(comb2_type).name());
	comb3_types seqs;
	return true;
}


