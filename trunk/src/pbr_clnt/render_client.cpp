#include "render_client.hpp"


namespace ma{
render_client* render_client::self = 0;

	void render_client::start_render_scene(const std::string& file)
	{
		if (!(lua_.run_file(file)))
		{
			OOLUA::lua_stack_dump(lua_);
		}
		if (!(lua_ && lua_.call("startup",true)))
		{
			OOLUA::lua_stack_dump(lua_);
		}
	}
	void render_client::end_render_scene()
	{
		if(!(lua_ && lua_.call("cleanup")))
		{
			OOLUA::lua_stack_dump(lua_);
		}
	
	}
}
