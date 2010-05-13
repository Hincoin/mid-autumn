#include "simple_test_framework.hpp"

#include <map>
#include <string>
#include <stdio.h>

typedef std::map<std::string,test_func_t> function_map_t;

static function_map_t& get_functions()
{
	static function_map_t functions;
	return functions;
}
bool register_test_function(const char* module_name,test_func_t f){
	if (get_functions().find(module_name) != get_functions().end()) 
	{
		assert(false);
		return false;
	}
	get_functions().insert(std::make_pair(module_name,f));
	return true;
}
bool run_test(int argc,char** argv){
	bool result = true;
	if(argc == 1)
	for (function_map_t::iterator i = get_functions().begin();i  != get_functions().end(); ++i)
	{
		printf("run test module %s ",i->first.c_str());
		if((*(i->second))())
		{printf("success !");}
		else
		{printf("failed !");result = false;}
		printf("\n");
	}
	else{
		for (int i = 1;i < argc; ++i)
		{
			function_map_t::iterator it = get_functions().find(argv[i]);
			if(it != get_functions().end())
			{
				printf("run test module %s ",it->first.c_str());
				if((*(it->second))())
				{printf("success !");}
				else
				{printf("failed !");result = false;}
				printf("\n");
			}
		}
	}
	return result;
}
