#include "pbr_ctrl.hpp"

using namespace ma;
int main(int argc,char* argv[])
{

	OOLUA::Script lua ;
	register_functions(lua);

	try
	{
		if(argc != 4)
		{
			std::cerr << "Usage: pbr_ctrl <script> <host> <port>"<<std::endl;
			return 1;
		}
		lua.run_file(argv[1]);
		boost::asio::io_service io_service;
		pbr_ctrl ctrl(lua,io_service,argv[2],argv[3]);
		io_service.run();

	}
	catch(std::exception& e)
	{std::cerr<<e.what()<<std::endl; return 1;}
	return 0;
}
