#include <boost/mpl/transform.hpp>

#include "simple_test_framework.hpp"


bool class_combination_test();

REGISTER_TEST_FUNC(class_combination_test,class_combination_test)

#include <boost/fusion/container.hpp>
#include <boost/fusion/adapted/mpl.hpp>


#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/container/vector/vector_fwd.hpp>
#include <boost/fusion/include/vector_fwd.hpp>


#include <stdio.h>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/list.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/equal.hpp>

#include <boost/unordered_map.hpp>

template<template<typename MPL_Seq>class ToFusionSeq,template<typename Arg>class Fun,typename Seq>
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
template<
template<typename MPL_Seq>class ToFusionSeq
,template<typename Arg1,typename Arg2>class Fun2
,typename Seq0,typename Seq1
>
struct combination2{
	template<typename T>
	struct combination2_helper{
		template<typename U0>
			struct reduced:Fun2<T,U0>{};
		typedef typename combination1<ToFusionSeq,reduced,Seq1>::type type;
		//typedef typename combination1<Seq1,reduced>::fusion_seqence fusion_seqence;
	};
	typedef typename boost::mpl::transform<Seq0,combination2_helper<boost::mpl::_1> >::type mpl_type;
	//typedef combination1<Seq0	
	typedef typename ToFusionSeq<mpl_type>::type type;


};

template<template<typename MPL_Seq>class ToFusionSeq
,template<typename Arg1,typename Arg2,typename Arg3>class Fun3
,typename Seq0,typename Seq1,typename Seq2>
	struct combination3{
		template<typename T>
		struct combination3_helper{
			template<typename U0,typename U1>
			struct reduced:Fun3<T,U0,U1>{};
			typedef typename combination2<ToFusionSeq,reduced,Seq1,Seq2>::type type;	
		};	
		typedef typename boost::mpl::transform<Seq0,combination3_helper<boost::mpl::_1> >::type mpl_type;
		typedef typename ToFusionSeq<mpl_type>::type type;	
	};
#include <boost/mpl/joint_view.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/copy.hpp>
#include <boost/mpl/back_inserter.hpp>

	namespace mpl = boost::mpl;
	template<typename Comb>
	struct peek_sequence: mpl::and_<mpl::is_sequence<Comb> ,mpl::not_<mpl::empty<Comb> >, mpl::is_sequence<typename mpl::front<Comb>::type > >{};
	template<typename Comb,bool>
	struct combination_to_seq_impl;
	template<typename Comb>
	struct combination_to_seq_impl<Comb,true>{
		typedef typename mpl::front<Comb>::type car;
		typedef typename mpl::pop_front<Comb>::type cdr;
		
		typedef mpl::joint_view<
			typename combination_to_seq_impl<car,peek_sequence<car>::value>::type
			,typename combination_to_seq_impl<cdr,peek_sequence<cdr>::value>::type
		> type;
	};
	template<typename Comb>
	struct combination_to_seq_impl<Comb,false>{
		typedef Comb type;
	};
	template<typename Combination,typename ResultSeq>
	struct combination_to_sequence{
		//BOOST_MPL_ASSERT((peek_sequence<Combination>));
		typedef typename combination_to_seq_impl<Combination,peek_sequence<Combination>::value>::type seq_view;
		typedef typename mpl::copy<seq_view,mpl::back_inserter<ResultSeq> >::type type;
	};
	template<typename T>
	struct map_type_str;
#define MAP_TYPE_STR(TYPE,TYPE_STR)\
	template<>\
	struct map_type_str<TYPE>{\
	static const char* type_str(){return #TYPE_STR;}\
	static const size_t type_str_size = sizeof(#TYPE_STR);\
	typedef TYPE type;\
	};\

	template<typename T>
	struct to_map_type_str{
		typedef map_type_str<T> type;
	};

	template<typename Seq>
	struct to_mapped_sequence{
		typedef typename boost::mpl::transform<Seq,to_map_type_str<boost::mpl::_1> >::type type;
	};


#include <string>
	typedef int (*test_func)();
	boost::unordered_map<std::string,test_func> test_func_map;

	template<typename T0,typename T1,typename T2>
	struct func{
		typedef func<T0,T1,T2> type;
		func(){
			std::string t(map_type_str<T0>::type_str());
			t+=map_type_str<T1>::type_str();
			t+=map_type_str<T2>::type_str();
			test_func_map.insert(std::make_pair(t,&type::make_test));
			printf("type_str_size :%d\n",type_str_size);
		}
		static const size_t type_str_size = map_type_str<T0>::type_str_size + map_type_str<T1>::type_str_size + map_type_str<T2>::type_str_size-3+1 ;
		static const char* type_str(){
			static char type_str_ [type_str_size];
		}
		static int make_test(){	
			return 0;
		};
	};
	template<typename T0,typename T1,typename T2>
	struct map_type_str<func<T0,T1,T2> >:func<T0,T1,T2>{
		//... type_str and type_str_size
	};
	




















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

MAP_TYPE_STR(int,int);
MAP_TYPE_STR(float,float);
MAP_TYPE_STR(A,A);
MAP_TYPE_STR(B,B);
MAP_TYPE_STR(C,C);
MAP_TYPE_STR(D,D);
MAP_TYPE_STR(E,E);
MAP_TYPE_STR(F,F);
#include <typeinfo>


//testing code for my application
typedef float scalar_t;
typedef int spectrum_t; //fake it

template<typename T>
struct const_texture{};

template<typename T,typename Mapping>
struct uvtexture{};

struct map2d{};
struct spherical_map2d{};
struct cylinder_map2d{};
struct planar_map2d{};
struct map3d{};
struct identity_map3d{};
template<typename ScalarTex,typename SpectrumTex>
struct matte_mtl{};

struct Sphere{};
struct Polygon{};
struct Triangle{};
struct TriangleMesh{};

template<typename Mtl,typename Shape>
struct primitive{};


bool class_combination_test()
{
	///using namespace boost;
	typedef mpl::vector<A,B> seq_type1;
	typedef mpl::vector<C,D> seq_type2;
	typedef mpl::vector<E,F> seq_type3;
	typedef mpl::vector<int,float> seq_type4;

	typedef combination3<mpl::identity,func,seq_type1,seq_type2,seq_type3>::type comb3_types;
	typedef combination_to_sequence<comb3_types,mpl::vector<> >::type seq_types;
	comb3_types seqs;

	printf("combination3 string: %s \n",typeid(seq_types).name());
	return true;
}


