#ifndef MA_INCLUDE_PARAMSET_HPP
#define MA_INCLUDE_PARAMSET_HPP

#include "MAny.hpp"
#include "AssocVector.hpp"
#include <string>

class ParamSet
{
public:
	typedef AssocVector<std::string,ma::MAny> paramerter_set;

	template<typename T>
	const T& as(const std::string& name,const T& default_value=T()  )const
	{
		paramerter_set::const_iterator it = parameters_.find(name);
		return it == parameters_.end()? default_value : it->second.template cast<T>();
	}
	template<typename T>
	void add(const std::string& name,const T& x){parameters_.insert(std::make_pair(name,x));}
protected:
private:
	paramerter_set parameters_;
};


#endif