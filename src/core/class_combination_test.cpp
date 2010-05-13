#include <boost/mpl/transform.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/joint_view.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/mpl/for_each.hpp>

#include "simple_test_framework.hpp"

#include "CartesianProductView.hpp"
#include "TypeMap.hpp"
bool class_combination_test();

REGISTER_TEST_FUNC(class_combination_test,class_combination_test)

	
#include <typeinfo>
#include <string>
#include <vector>
#include "ParamSet.hpp"
#include "variant_creator.hpp"
#include <iostream>
using namespace std;
using namespace ma;

namespace mpl = boost::mpl;

type_name2code name2code;

#define MAKE_TYPE_CODE(N,TYPE,NAME)\
	MAKE_TYPE_STR_MAP(N,TYPE,NAME,name2code)

//testing code for my application
typedef float scalar_t;
typedef int spectrum_t; //fake it
MAKE_TYPE_CODE(0,scalar_t,scalar_t);
MAKE_TYPE_CODE(0,spectrum_t,color);

typedef mpl::vector<scalar_t,spectrum_t> float_color;

template<typename T>
struct const_texture{typedef const_texture<T> type;};

template<typename T,typename Mapping>
struct uvtexture{typedef uvtexture<T,Mapping> type;};

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

MAKE_TYPE_CODE(0,map2d,map2d);
MAKE_TYPE_CODE(0,map3d,map3d);
MAKE_TYPE_CODE(0,spherical_map2d,spherical_map2d);
MAKE_TYPE_CODE(0,cylinder_map2d,cylinder_map2d);
MAKE_TYPE_CODE(0,planar_map2d,planar_map2d);
MAKE_TYPE_CODE(0,identity_map3d,identity_map3d);

typedef mpl::vector<map2d/*,map3d,identity_map3d,spherical_map2d,cylinder_map2d,planar_map2d*/> map_type_seq;

template<typename ScalarTex,typename SpectrumTex>
struct matte_mtl{
	typedef matte_mtl<ScalarTex,SpectrumTex> type;
	matte_mtl(const ScalarTex*,const SpectrumTex*){}
};
MAKE_TYPE_CODE(2,matte_mtl,matte);

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
MAKE_TYPE_CODE(0,Triangle,Triangle);
MAKE_TYPE_CODE(0,TriangleMesh,TriangleMesh);
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
typedef primitive* (*primitive_maker_t)(const ParamSet&,const ParamSet&);
typedef MAny (*texture_maker_t)(const ParamSet&);
typedef boost::variant<primitive_maker_t,texture_maker_t> maker_t;
boost::unordered_map<type_code_comp_t,maker_t> type_maker;
boost::unordered_map<string,type_code_comp_t> texname_typecode;//only reference to maker
boost::unordered_map<string,MAny> texname_textures;
//texture name to type
//texture params,
template<typename T> struct PrimitiveCreator;
template<typename Mtl,typename Shape>
struct PrimitiveCreator<geo_primitive<Mtl,Shape> >{
	PrimitiveCreator(){
	}
	static primitive* create(const ParamSet& mtl_param,const ParamSet& shape_param){
		Mtl* m = create_material<Mtl>()(mtl_param);
		Shape* s = create_shape<Shape>()(shape_param);
		return new geo_primitive<Mtl,Shape>(m,s);
	}
};
REGISTER_TYPE_STR_MAKER_MAP(2,geo_primitive,geometry,name2code,PrimitiveCreator,type_maker);

template<typename T>
struct TextureCreator{
	static MAny create(const ParamSet& p)	
	{
		const string& name = p.as<string>("name");
		texname_typecode[name]=type_code_comp_t(map_type_str<T>::type_code(),map_type_str<T>::type_code()+map_type_str<T>::type_code_size_);
		boost::unordered_map<string,MAny>::iterator it = texname_textures.find(name);
		if (it != texname_textures.end())
			return it->second;
		MAny r(create_texture<T>()(p));
		texname_textures.insert(std::make_pair(name,r));
		return r;
	}
	TextureCreator(){
	}
};
REGISTER_TYPE_STR_MAKER_MAP(1,const_texture,const_texture,name2code,TextureCreator,type_maker);
REGISTER_TYPE_STR_MAKER_MAP(2,uvtexture,uvtexture,name2code,TextureCreator,type_maker);


template<typename S>
struct make_uv_texture_t{
	typedef 
	typename mpl::at<S,mpl::int_<0> >::type s_t;
	typedef
			 typename mpl::at<S,mpl::int_<1> >::type m_t;
typedef uvtexture<s_t,m_t
				 > type;
};

template<typename S>
struct make_matte_t{
typedef typename matte_mtl<typename mpl::at<S,mpl::int_<0> >::type,typename mpl::at<S,mpl::int_<1> >::type>::type type;
};
template<typename T>
struct texture_seq{
typedef mpl::vector<T> seq;
typedef typename mpl::transform_view<seq,const_texture<mpl::_1> >::type const_texture_seq;
typedef typename mpl::product_view<mpl::vector<seq,map_type_seq> >::type product_of_uv_param;
typedef typename mpl::transform_view<product_of_uv_param,make_uv_texture_t<mpl::_1> >::type uv_texture_seq;
typedef typename mpl::joint_view<const_texture_seq,uv_texture_seq>::type texture_view;
typedef texture_view  type;	
};

struct primitive_iteration{
template<typename Seq>
	void operator()(Seq){
		typedef typename mpl::at_c<Seq,0>::type mtl_type;
		typedef typename mpl::at_c<Seq,1>::type shape_type;
		typedef geo_primitive<mtl_type,shape_type> geometry;
		map_type_str<geometry>::type_code();

		type_code_comp_t real_compare(map_type_str<geometry>::type_code(),map_type_str<geometry>::type_code_size_ + map_type_str<geometry>::type_code());
		real_compare;
	}

};
primitive* make_primitive(const ParamSet& mtl_param,const ParamSet& shape_param,const string& primitive_type)
{
	const	string& scalar_tex = mtl_param.as<string>("scalar_tex");
	const string& color_tex = mtl_param.as<string>("color_tex");
	const string& mat_type = mtl_param.as<string>("material_type");
	const string& shape_type = shape_param.as<string>("shape_type");

	type_code_comp_t type_codes = texname_typecode[scalar_tex];
	std::copy(texname_typecode[color_tex].begin(),
			texname_typecode[color_tex].end(),
			std::back_inserter(type_codes));
	type_codes.push_back(name2code[mat_type]);
	type_codes.push_back(name2code[shape_type]);
	type_codes.push_back(name2code[primitive_type]);
	typedef    	map_type_str<geo_primitive<matte_mtl<const_texture<scalar_t>,uvtexture<spectrum_t,map2d> >,Triangle> > mapped_geometry;
	type_code_comp_t another_compare;
	another_compare.push_back(name2code["scalar_t"]);
	another_compare.push_back(name2code["const_texture"]);
	another_compare.push_back(name2code["color"]);
	another_compare.push_back(name2code["map2d"]);
	another_compare.push_back(name2code["uvtexture"]);
	another_compare.push_back(name2code["matte"]);
	another_compare.push_back(name2code["Triangle"]);
	another_compare.push_back(name2code["geometry"]);
	type_code_comp_t real_compare(mapped_geometry::type_code(),mapped_geometry::type_code_size_ + mapped_geometry::type_code()) ;
	
	primitive_maker_t m = get_creator<primitive_maker_t>(type_codes,type_maker);
	primitive_maker_t mm = get_creator<primitive_maker_t>(real_compare,type_maker);
	primitive_maker_t mmm = get_creator<primitive_maker_t>(another_compare,type_maker);
	if (m)return m(mtl_param,shape_param);
	return 0;
}
//bool test_make_primitive()
//{
//	//initialize types
//	typedef mpl::vector<scalar_t> float_t;
//	typedef mpl::vector<spectrum_t> color_t;
//	typedef mpl::joint_view<texture_seq<scalar_t>::type,texture_seq<spectrum_t>::type> textures_view;
//	typedef mpl::product_view<mpl::vector<texture_seq<scalar_t>::type,texture_seq<spectrum_t>::type> >::type product_of_matte_param;
//	typedef mpl::transform_view<product_of_matte_param,make_matte_t<mpl::_1> >::type matte_view;
//	//typedef mpl::copy<matte_view,mpl::back_inserter<mpl::vector<> > >::type matte_seq;
//	//matte_seq x;
//	typedef mpl::product_view<mpl::vector<matte_view,shape_seq> >::type geometry_param;
//	mpl::for_each<geometry_param>(primitive_iteration());
//	//matte_comb_tuple register_matte_types;
//	///////////////////////////////////////////////
//	//create textures
//	type_code_comp_t tex_code;
//	tex_code.push_back(name2code["scalar_t"]);	
//	tex_code.push_back(name2code["const_texture"]);
//	ParamSet tex_param1;
//	tex_param1.add("name",string("scalar_tex_name"));
//	texture_maker_t m = get_creator<texture_maker_t>(tex_code,type_maker);
//	if (m) m(tex_param1);
//	tex_code.clear();
//	tex_code.push_back(name2code["color"]);
//	tex_code.push_back(name2code["map2d"]);
//	tex_code.push_back(name2code["uvtexture"]);
//	ParamSet tex_param2;
//	tex_param2.add("name",string("color_tex_name"));
//	m = get_creator<texture_maker_t>(tex_code,type_maker);
//	if (m) m(tex_param2);
//
//
//	//////////////////////////////////////////////
//	ParamSet mtl_params;
//	mtl_params.add("scalar_tex",string("scalar_tex_name"));
//	mtl_params.add("color_tex",string("color_tex_name"));
//	mtl_params.add("material_type",string("matte"));
//	ParamSet shape_params;
//	shape_params.add("shape_type",string("Triangle"));
//	
//	primitive* primitive_result = make_primitive(mtl_params,shape_params,"geometry");
//	return primitive_result != 0;
//
//}

bool class_combination_test()
{
	//test_make_primitive();
	
	return true;
}


