#ifndef MA_INCLUDE_PARAMSET_HPP
#define MA_INCLUDE_PARAMSET_HPP

#include <string>

#include "MAny.hpp"

#include "AssocVector.hpp"


class ParamSet
{
public:
	typedef AssocVector<std::string,ma::MAny> paramerter_set;

	template<typename T>
	typename boost::mpl::if_<boost::is_pointer<T>,
		const typename boost::remove_pointer<T>::type*,
		const T&>::type
	as(const std::string& name,const T& default_value=T()  )const
	{
		paramerter_set::const_iterator it = parameters_.find(name);
		//BOOST_MPL_ASSERT((boost::is_pointer<T>));
		return it == parameters_.end()? default_value : it->second.template cast<T>();
		//return result;
	}
	template<typename T>
	void add(const std::string& name,const T& x){parameters_.insert(std::make_pair(name,x));}
protected:
private:
	paramerter_set parameters_;
};


#endif
