#include <fstream>
#include "rpc_ctrl.hpp"

namespace ma{
	namespace rpc{
	namespace s2ctrl{
	void write_image(std::string file,std::string data)
	{
		std::ofstream out(file.c_str(),std::ios::out);	
		out<<data;
	}
	}
	}
}


