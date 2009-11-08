#include <boost/variant.hpp>
#include "ParamSet.hpp"
#include <vector>
#include <boost/type_traits.hpp>
#include <cassert>

namespace ma{
template<typename Func>
struct object_creator{
typedef typename boost::make_recursive_variant<Func,
		std::vector<boost::recursive_variant_>
			>::type creator_variant_t;
struct get_creator:public boost::static_visitor<Func>
	{
		const size_t* idx_;
		size_t size;
		get_creator(const size_t* idx,size_t s):idx_(idx),size(s){}
		Func operator()(const Func& f)const
		{
			assert(size == 0);
			return f;
		}
		Func operator()(const std::vector<creator_variant_t>& v)const
		{
			assert(size > 0);
			return boost::apply_visitor(get_creator(idx_+1,size-1),(v[*idx_]));	
		}	

	};

	static Func get_create_function(const size_t* idx,size_t s,const creator_variant_t& v)
	{
		return boost::apply_visitor(get_creator(idx+1,s),v);	
	}
};
}

#include <boost/unordered_map.hpp>
#include "Singleton.hpp"

namespace ma{

	typedef boost::unordered_map<std::string, size_t > map_type_str_id_t;
	template<typename FuncT>
		struct Creators{
			typedef typename object_creator<FuncT>::creator_variant_t creator_variant_t;			
			std::vector<creator_variant_t> creators_;
		};

}


