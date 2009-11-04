#include <boost/mpl/transform.hpp>

#include "simple_test_framework.hpp"


bool class_combination_test();

REGISTER_TEST_FUNC(class_combination_test,class_combination_test)

#define FUSION_MAX_VECTOR_SIZE 50
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
#include <boost/mpl/accumulate.hpp>
#include <boost/unordered_map.hpp>

#include <boost/mpl/transform_view.hpp>
#include <boost/mpl/filter_view.hpp>
#include <boost/mpl/joint_view.hpp>
#include <boost/mpl/empty_sequence.hpp>
template<template<typename Arg>class Fun,typename Seq>
struct combination1{
	typedef typename boost::mpl::transform<Seq,Fun<boost::mpl::_1> >::type mpl_type;
	typedef mpl_type type;
};
template<
template<typename Arg1,typename Arg2>class Fun2
,typename Seq0,typename Seq1
>
struct combination2{
	template<typename T>
	struct combination2_helper{
		template<typename U0>
			struct reduced{
				typedef typename Fun2<T,U0>::type type;
			};
		typedef typename combination1<reduced,Seq1>::type type;
	};
	typedef typename boost::mpl::transform<Seq0,combination2_helper<boost::mpl::_1> >::type mpl_type;
	typedef mpl_type type;

};

template<
template<typename Arg1,typename Arg2,typename Arg3>class Fun3
,typename Seq0,typename Seq1,typename Seq2>
	struct combination3{
		template<typename T>
		struct combination3_helper{
			template<typename U0,typename U1>
			struct reduced{
				typedef typename Fun3<T,U0,U1>::type type;	
			};
			typedef typename combination2<reduced,Seq1,Seq2>::type type;	
		};	
		typedef typename boost::mpl::transform<Seq0,combination3_helper<boost::mpl::_1> >::type mpl_type;
		typedef mpl_type type;	
	};

	//////////////////////////////////////////////////////////////////////////

	template<template<typename Arg>class Fun,typename Seq>
	struct combination1_view{
		typedef typename boost::mpl::transform_view<Seq,Fun<boost::mpl::_1> >::type type;
	};
	template<
template<typename Arg1,typename Arg2>class Fun2
		,typename Seq0,typename Seq1
	>
	struct combination2_view{
		template<typename T>
		struct combination2_helper{
			template<typename U0>
			struct reduced{
				typedef typename Fun2<T,U0>::type type;	
			};
			typedef typename combination1_view<reduced,Seq1>::type type;
		};
		typedef typename boost::mpl::transform_view<Seq0,combination2_helper<boost::mpl::_1> >::type mpl_type;
		//joint_view for every mpl_type ...
		//accumulate ....
		typedef typename boost::mpl::accumulate<mpl_type,
			boost::mpl::empty_sequence ,
			boost::mpl::joint_view<boost::mpl::_1,boost::mpl::_2> >::type mpl_seqence_view;
		typedef mpl_seqence_view type;


	};

	template<template<typename Arg1,typename Arg2,typename Arg3>class Fun3
		,typename Seq0,typename Seq1,typename Seq2>
	struct combination3_view{
		template<typename T>
		struct combination3_helper{
			template<typename U0,typename U1>
			struct reduced{
				typedef typename Fun3<T,U0,U1>::type type;	
			};
			typedef typename combination2_view<reduced,Seq1,Seq2>::type type;	
		};	
		typedef typename boost::mpl::transform_view<Seq0,combination3_helper<boost::mpl::_1> >::type mpl_type;
		//joint_view for every mpl_type ...
		//accumulate ....
		typedef typename boost::mpl::accumulate<mpl_type,
			boost::mpl::empty_sequence ,
			boost::mpl::joint_view<boost::mpl::_1,boost::mpl::_2> >::type mpl_seqence_view;
		typedef mpl_seqence_view type;	
	};
	//////////////////////////////////////////////////////////////////////////

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
	template<typename Combination>
	struct combination_to_sequence{
		//BOOST_MPL_ASSERT((peek_sequence<Combination>));
		typedef typename combination_to_seq_impl<Combination,peek_sequence<Combination>::value>::type type;
	};
	template<typename T>
	struct map_type_str
{
	typedef T type;
	static const char* type_str(){return T::type_str();}
	static const size_t type_str_size = T::type_str_size;
};

#define MAP_TYPE_STR(TYPE,TYPE_STR)\
	template<>\
	struct map_type_str<TYPE>{\
	static const char* type_str(){return #TYPE_STR;}\
	static const size_t type_str_size = sizeof(#TYPE_STR);\
	typedef TYPE type;\
	};\


#include <string>
	typedef int (*test_func)();
	boost::unordered_map<std::string,test_func> test_func_map;

	template<typename T0,typename T1,typename T2>
	struct func3{
		typedef func3<T0,T1,T2> type;
	};
	template<typename T0,typename T1>
	struct func2{
		typedef func2<T0,T1> type;
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

#include <typeinfo>
#include <string>
#include <vector>
#include "ParamSet.hpp"
#include "variant_creator.hpp"
#include <iostream>
using namespace std;
//testing code for my application
typedef float scalar_t;
typedef int spectrum_t; //fake it
MAP_TYPE_STR(scalar_t,float);
MAP_TYPE_STR(spectrum_t,color);

typedef mpl::vector<scalar_t,spectrum_t> float_color;

template<typename T>
struct const_texture{typedef const_texture<T> type;};

template<typename T,typename Mapping>
struct uvtexture{typedef uvtexture<T,Mapping> type;};

template<typename T>
struct map_type_str<const_texture<T> >{
	static const size_t type_str_size = map_type_str<T>::type_str_size - 1 + sizeof("const_texture");
	char type_str_[type_str_size];
	typedef const_texture<T> type;
	map_type_str(){
		::memset(type_str_,0,type_str_size);
		::strcpy(type_str_,map_type_str<T>::type_str());
		::strcpy(type_str_ + map_type_str<T>::type_str_size - 1,"const_texture");
	}
	static const char* type_str()
	{
		static map_type_str self;
		return self.type_str_;
	}

};
template<typename T,typename Mapping>
struct map_type_str<uvtexture<T,Mapping> >
{
	static const size_t type_str_size = map_type_str<T>::type_str_size + map_type_str<Mapping>::type_str_size - 2 +
		sizeof("uvtexture");
	char type_str_[type_str_size];
	typedef uvtexture<T,Mapping> type;

	map_type_str(){
		::memset(type_str_,0,type_str_size);

		::strcpy(type_str_,map_type_str<T>::type_str());
		::strcpy(type_str_ + map_type_str<T>::type_str_size - 1,map_type_str<Mapping>::type_str());
		::strcpy(type_str_ + map_type_str<T>::type_str_size + map_type_str<Mapping>::type_str_size - 2,
				"uvtexture");

	}
	static const char* type_str()
	{
		static map_type_str self;
		return self.type_str_;
	}
};
template<typename T>
struct create_texture;

template<typename T>
struct create_texture<const_texture<T> >
{
	const_texture<T>* operator()(const ParamSet& )const{return new const_texture<T>();}
};
template<typename T,typename Mapping>
struct create_texture<uvtexture<T,Mapping> >{

	uvtexture<T,Mapping>* operator()(const ParamSet& param)const
	{
		const string& mapping_type = param.as<string>("mapping_type");		
		//do somting ....

		return new uvtexture<T,Mapping>();
	}

};
struct map2d{};
struct spherical_map2d{};
struct cylinder_map2d{};
struct planar_map2d{};
struct map3d{};
struct identity_map3d{};

MAP_TYPE_STR(map2d,map2d);
MAP_TYPE_STR(map3d,map3d);
MAP_TYPE_STR(spherical_map2d,spherical_map2d);
MAP_TYPE_STR(cylinder_map2d,cylinder_map2d);
MAP_TYPE_STR(planar_map2d,planar_map2d);
MAP_TYPE_STR(identity_map3d,identity_map3d);
typedef mpl::vector<map2d/*,map3d,identity_map3d,spherical_map2d,cylinder_map2d,planar_map2d*/> map_type_seq;

template<typename ScalarTex,typename SpectrumTex>
struct matte_mtl{
	typedef matte_mtl<ScalarTex,SpectrumTex> type;
	matte_mtl(const ScalarTex*,const SpectrumTex*){}
};
template<typename Sc,typename Sp>
struct map_type_str<matte_mtl<Sc,Sp> >{
	static const size_t type_str_size = map_type_str<Sc>::type_str_size + map_type_str<Sp>::type_str_size -2 + sizeof("matte");
	char type_str_[type_str_size];
	typedef matte_mtl<Sc,Sp> type;
	map_type_str(){
		::memset(type_str_,0,type_str_size);

		::strcpy(type_str_,map_type_str<Sc>::type_str());
		::strcpy(type_str_ + map_type_str<Sc>::type_str_size - 1,map_type_str<Sp>::type_str());
		::strcpy(type_str_ + map_type_str<Sc>::type_str_size + map_type_str<Sp>::type_str_size - 2,
				"matte");
		printf("matte: %s \n",type_str_);
	}
	static const char* type_str(){
		static map_type_str self;
		return self.type_str_;
	}

};

template<typename T>
struct matte_mtl_creator{
typedef map_type_str<T> type;
};
template<typename Mtl>
struct create_material;
template<typename ScalarTex,typename SpectrumTex>
struct create_material<matte_mtl<ScalarTex,SpectrumTex> >{
	
	matte_mtl<ScalarTex,SpectrumTex>*
		operator()(const ParamSet& params){
			//const string& s_tex_name = params.as<string>("kd");
			//const string& sp_tex_name = params.as<string>("sigma");
			//find in global 	
			const ScalarTex* s_tex = params.as<ScalarTex*>("kd");
			const SpectrumTex* sp_tex = params.as<SpectrumTex*>("sigma");

			return new matte_mtl<ScalarTex,SpectrumTex>(s_tex,sp_tex);
		}

};

struct Triangle{};
struct TriangleMesh{};
MAP_TYPE_STR(Triangle,Triangle);
MAP_TYPE_STR(TriangleMesh,TriangleMesh);
typedef mpl::vector<TriangleMesh,Triangle> shape_seq;

template<typename Shape>
struct create_shape
{
	Shape* operator()(const ParamSet& ){return new Shape();}

};
struct primitive{
	virtual ~primitive(){}
};
template<typename Mtl,typename Shape>
struct geo_primitive:primitive{
	typedef geo_primitive<Mtl,Shape> type;
	geo_primitive(Mtl* m,Shape* s){}
};

using std::string;
using namespace ma;
using std::vector;
//managed by global state 
boost::unordered_map<string,const char*> texname_type;
typedef primitive* (*primitive_maker_t)(const ParamSet&,const ParamSet&);
typedef MAny (*texture_maker_t)(const ParamSet&);
boost::unordered_map<string,primitive_maker_t> primitive_maker;
boost::unordered_map<string,texture_maker_t> textures_maker;
boost::unordered_map<string,MAny> texname_textures;
//texture name to type
//texture params,
template<typename Mtl,typename Shape>
struct PrimitiveCreator{
	typedef map_type_str<Mtl> mtl_type_str;
	typedef map_type_str<Shape> shape_type_str;
	typedef PrimitiveCreator<Mtl,Shape> type;
	static const size_t type_str_size = mtl_type_str::type_str_size + shape_type_str::type_str_size - 2 + sizeof("geometry");
	static char type_str_[type::type_str_size];
	PrimitiveCreator(){
		::memset(type_str_,0,type_str_size);

		string key=mtl_type_str::type_str();
		key += shape_type_str::type_str();
		key += "geometry";
		::strcpy(type_str_,key.c_str());
		
		primitive_maker.insert(std::make_pair(key,&PrimitiveCreator::make));	
	}
	static primitive* make(const ParamSet& mtl_param,const ParamSet& shape_param){
		Mtl* m = create_material<Mtl>()(mtl_param);
		Shape* s = create_shape<Shape>()(shape_param);
		return new geo_primitive<Mtl,Shape>(m,s);
	}
	static const char* type_str()
	{
		static type self;
		return self.type_str_;	
	}

};

template<typename T>
struct TextureCreator{
	typedef map_type_str<T> texture_type_str;
	typedef TextureCreator<T> type;
	static const size_t type_str_size = texture_type_str::type_str_size;
     char type_str_[type_str_size];	
	static MAny make(const ParamSet& p)	
	{
		const string& name = p.as<string>("name");
		boost::unordered_map<string,MAny>::iterator it = texname_textures.find(name);
		if (it != texname_textures.end())
			return it->second;
		MAny r(create_texture<T>()(p));
		texname_textures.insert(std::make_pair(name,r));
		return r;
	}
	TextureCreator(){
		::memset(type_str_,0,type_str_size);	
		::strcpy(type_str_,texture_type_str::type_str());
		textures_maker.insert(std::make_pair(type_str_,&type::make));
	}
	static const char* type_str()
	{ static type self;return self.type_str_;}

};
template<typename Mtl,typename Shape>
char PrimitiveCreator<Mtl,Shape>::type_str_[PrimitiveCreator<Mtl,Shape>::type_str_size]  = {0};
primitive* make_primitive(const ParamSet& mtl_params,const ParamSet& shape_params)
{
	//materialtypes<texture types> + shape_types
	//get mtl_texture_names to get texture types
	//string mtl_type = mtl_params.as<string>("type");
	const std::vector<string>& texture_names = mtl_params.as<vector<string> >("texture_names");
	string mtl_type_texture_types;
	for(vector<string>::const_iterator it = texture_names.begin();
			it != texture_names.end();++it)
	{
		boost::unordered_map<string,const char*>::iterator finded =
			texname_type.find(*it);
		if (finded != texname_type.end())
			mtl_type_texture_types+=finded->second; 
	}
	mtl_type_texture_types += mtl_params.as<string>("type");

	const string& shape_type = shape_params.as<string>("type");
	boost::unordered_map<string,primitive_maker_t>::iterator it =
	primitive_maker.find(mtl_type_texture_types + shape_type + "geometry");
	if (it != primitive_maker.end())
		return	(*(it->second))(mtl_params,shape_params);	
	return 0;

}
void make_mapping(const string& mapping_type,ParamSet& tex_param)
{
	tex_param.add("mapping_type",mapping_type);
}
void make_texture(const string& tex_type,const string& name,const string& mapping_type,ParamSet& mtl_params)
{
	texname_type.insert(std::make_pair(name,tex_type.c_str()));	
	ParamSet tex_params;
	make_mapping(mapping_type,tex_params);
	tex_params.add("name",name);
	boost::unordered_map<string,texture_maker_t>::iterator it = textures_maker.find(tex_type);
	if (it != textures_maker.end())
		mtl_params.add(name,(*(it->second))(tex_params));
	
}
//template<typename T>
//struct texture_seq{
//	typedef mpl::vector<T> seq;
//	typedef typename combination1<mpl::identity,const_texture,seq>::type const_texture_seq;
//	typedef typename combination2<mpl::identity,uvtexture,seq,map_type_seq>::type uv_texture_seq;
//	typedef typename mpl::joint_view<const_texture_seq,typename combination_to_sequence<uv_texture_seq>::type>::type texture_view;
//       	typedef typename mpl::copy<texture_view,mpl::back_inserter<mpl::vector<> > >::type type;	
//
//
//
//};

//
//bool test_make_primitive()
//{
//	//initialize types
//	typedef mpl::vector<scalar_t> float_t;
//	typedef mpl::vector<spectrum_t> color_t;
//	typedef mpl::joint_view<texture_seq<scalar_t>::type,texture_seq<spectrum_t>::type> textures_view;
//	typedef mpl::transform_view<textures_view,TextureCreator<mpl::_1> >::type textures_creator_view;
//	typedef boost::fusion::result_of::as_vector<textures_creator_view>::type textures_creator_tuple;
//	typedef combination2<mpl::identity,matte_mtl,texture_seq<scalar_t>::type,texture_seq<spectrum_t>::type >::type matte_comb;
//	typedef combination_to_sequence<matte_comb>::type matte_view;
//	typedef mpl::transform_view<matte_view,matte_mtl_creator<mpl::_1> >::type matte_creator_view;
//	typedef boost::fusion::result_of::as_vector<matte_creator_view>::type matte_comb_tuple;
//	typedef mpl::copy<matte_view,mpl::back_inserter<mpl::vector<> > >::type matte_seq;
//	//typedef combination2<boost::fusion::result_of::as_vector,matte_mtl,texture_seq<scalar_t>::type,texture_seq<spectrum_t>::type >::type matte_comb_tuple;
//	typedef combination2<boost::fusion::result_of::as_vector,PrimitiveCreator,matte_seq,shape_seq>::type primitive_creator_comb;
//	textures_creator_tuple register_texture_creators;
//	primitive_creator_comb register_primitive_creators;
//	//matte_comb_tuple register_matte_types;
//	///////////////////////////////////////////////
//	ParamSet mtl_params;
//	ParamSet shape_params;
//
//	shape_params.add("type",string("TriangleMesh"));
//	//add textures 
//	mtl_params.add("type",string("matte"));	
//	std::vector<string> tex_names;
//	tex_names.push_back("kd");
//	tex_names.push_back("sigma");
//	std::vector<string> tex_types;
//	tex_types.push_back("floatconst_texture");
//	tex_types.push_back("colormap2duvtexture");
//	//mtl_params.add("kd","");
//	//mtl_params.add("sigma",);	
//	for (size_t i = 0;i < tex_names.size(); ++i)
//		make_texture(tex_types[i],tex_names[i],"map2d",mtl_params);
//	mtl_params.add("texture_names",tex_names);
//	for (boost::unordered_map<string,primitive_maker_t>::iterator it = primitive_maker.begin();
//			it != primitive_maker.end();
//			++it)
//	{
//		std::cout<<it->first<<std::endl;
//	}
//	primitive* primitive_result = make_primitive(mtl_params,shape_params);
//	return primitive_result != 0;
//
//}
template<typename T>
struct apply{
	template<typename U>
static void execute(U&){printf("apply: %s \n",typeid(T).name());}
};
template<typename Seq,template<typename T> class Apply>
struct recursive_execution;

template<typename Iter,typename EndIt,template<typename T> class Apply>
struct recusive_executeion_impl;
template<typename Iter,template<typename T> class Apply>
struct recusive_executeion_impl<Iter,Iter,Apply>{
	template<typename U>
	static void execute(U&){}
};
template<typename Iter,typename EndIt,template<typename T> class Apply>
struct recusive_executeion_impl{
	template<typename U>
static void execute(U& context)
{
	typedef typename mpl::if_<mpl::is_sequence<typename mpl::deref<Iter>::type>,
		recursive_execution<typename mpl::deref<Iter>::type,Apply>,
		Apply<typename mpl::deref<Iter>::type>
			>::type executer;
	executer::execute(context);
	recusive_executeion_impl<typename mpl::next<Iter>::type,EndIt,Apply>::execute(context);
}
};
template<typename Seq,template<typename T> class Apply>
struct recursive_execution:recusive_executeion_impl<typename mpl::begin<Seq>::type,typename mpl::end<Seq>::type,Apply>{
};


template<typename Seq,template<typename T> class Apply>
struct iterate_execution;

template<typename Iter,typename EndIt,template<typename T> class Apply>
struct iterate_execution_impl;
template<typename Iter,template<typename T> class Apply>
struct iterate_execution_impl<Iter,Iter,Apply>{
	static void execute(){}
};
template<typename Iter,typename EndIt,template<typename T> class Apply>
struct iterate_execution_impl{
	static void execute()
	{
		typedef Apply<typename mpl::deref<Iter>::type> current_executer;
		current_executer::execute();
		recusive_executeion_impl<typename mpl::next<Iter>::type,EndIt,Apply>::execute();
	}
};
template<typename Seq,template<typename T> class Apply>
struct iterate_execution:iterate_execution_impl<typename mpl::begin<Seq>::type,typename mpl::end<Seq>::type,Apply>{
};

bool class_combination_test()
{
	//test_make_primitive();
	///using namespace boost;
	typedef mpl::vector<char,float> seq_type1;
	typedef mpl::vector<short,double> seq_type2;
	typedef mpl::vector<long,int> seq_type3;
	
	typedef mpl::vector<int,float> seq_type4;
	typedef mpl::vector<int,float,char,short,long,double> seq_type6;

	//typedef combination3<mpl::identity,func,seq_type1,seq_type2,seq_type3>::type comb3_types;
	////typedef combination_to_sequence<comb3_types,mpl::vector<> >::type seq_types;
	//typedef combination_to_sequence<comb3_types>::type view_t;
	//BOOST_MPL_ASSERT((mpl::is_sequence<view_t>));
	////comb3_types seqs;
	//typedef boost::fusion::result_of::as_vector<view_t>::type combined_tuple_t;

	//printf("combination3 string: %s \n",typeid(combined_tuple_t).name());
	//typedef combination3_view<func3,seq_type1,seq_type2,seq_type3>::type viewed_combination_t;
	
	//typedef combination2_view<func2,seq_type6,seq_type6>::type recured_viewed_combination_t;
	//BOOST_MPL_ASSERT((boost::mpl::is_sequence<recured_viewed_combination_t>));
	//typedef combination2<func2,seq_type6,recured_viewed_combination_t>::type SixbySixbySix;
	//int a = mpl::size<recured_viewed_combination_t>::value;
	//recursive_execution<SixbySixbySix,apply>::execute(a);
	//iterate_execution<recured_viewed_combination_t,apply>::execute();

	//typedef combination3<boost::fusion::result_of::as_vector,func,seq_type1,seq_type2,seq_type3>::type comb3_types_t;
	//comb3_types_t b;
	return true;
}


