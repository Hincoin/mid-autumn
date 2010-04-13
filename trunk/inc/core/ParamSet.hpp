#ifndef MA_INCLUDE_PARAMSET_HPP
#define MA_INCLUDE_PARAMSET_HPP

#include <string>

#include "MAny.hpp"

#include "AssocVector.hpp"

namespace ma{
class ParamSet
{
public:
	typedef AssocVector<std::string,ma::MAny> parameter_set;
	typedef AssocVector<std::string,ma::MAny>::iterator iterator;
	typedef parameter_set::const_iterator const_iterator;

	template<typename T>
	typename boost::mpl::if_<boost::is_pointer<T>,
		const typename boost::remove_pointer<T>::type*,
		T>::type
	as(const std::string& name,T default_value = T() )const
	{
		const_iterator it = parameters_.find(name);
		//BOOST_MPL_ASSERT((boost::is_pointer<T>));
		return it == parameters_.end()? default_value : it->second.template cast<T>();
		//return result;
	}
	template<typename T>
	void add(const std::string& name,const T& x){parameters_.insert(std::make_pair(name,ma::MAny(x)));}
	iterator begin(){return parameters_.begin();}
	iterator end(){return parameters_.end();}
	const_iterator begin()const{return parameters_.begin();}
	const_iterator end()const {return parameters_.end();}
	bool empty()const{return parameters_.empty();}
	void clear(){parameters_.clear();}
protected:
private:
	parameter_set parameters_;
};
}

#endif
