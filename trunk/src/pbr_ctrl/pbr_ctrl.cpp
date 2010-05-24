
#include "oolua.h"
#include "pbr_ctrl.hpp"
#include "rpc_ctrl.hpp"

namespace ma{
	void render_scene(const char* scene_file)
	{
		pbr_ctrl::get_controller().render_scene(scene_file);
	}
}
namespace OOLUA{
	using namespace ma;
	LUA_EXPORT_FUNC(void(const char*), render_scene)
}
namespace ma
{
	pbr_ctrl* pbr_ctrl::self_ = 0;


	void pbr_ctrl::render_scene(const char* scene)
	{
		printf("render_scene %s\n",scene);
		rpc::send_rpc<rpc::c2s::rpc_render_scene>(net::connection_write_handler_ptr(new rpc::rpc_null_handler()),connection_,std::string(scene));
	}
	void register_functions(lua_State* l)
	{
		using namespace OOLUA;
		REGISTER_FUNC(l,render_scene);
	}
}
