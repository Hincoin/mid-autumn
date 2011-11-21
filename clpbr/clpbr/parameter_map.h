#ifndef _PARAMETER_MAP_H_
#define _PARAMETER_MAP_H_

#include <map>
#include <string>
#include "any.h"
class ParameterMap
{
public:
	typedef std::map<std::string,Any> map_t;
	typedef map_t::const_iterator const_iterator;

	void insert(const std::string& key, const Any& value){map_.insert(std::make_pair(key,value));}

	template<typename T>
	bool get(const std::string& key,T* value)const
	{
		const_iterator it = map_.find(key);
		if (it != map_.end())
		{
			const T* result = any_cast<T>(&it->second);
			if (result)
			{
				*value = *result;
				return true;
			}
		}
		return false;
	}
protected:
private:
	map_t map_;
};

#endif