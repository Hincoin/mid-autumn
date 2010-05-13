#ifndef _MA_INCLUDED_SCENE_PARSER_HPP_
#define _MA_INCLUDED_SCENE_PARSER_HPP_

#include "ParamSet.hpp"
#include <string>
namespace ma{
	ParamSet parseObjTriangleMesh(const std::string& file_name);
}


#endif