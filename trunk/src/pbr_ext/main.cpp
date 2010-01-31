#include "script.hpp"



int main(int argc,char** argv)
{
	OOLUA::Script lua ;
	register_api(lua);
	register_all_class(lua);
	return 0;
}


