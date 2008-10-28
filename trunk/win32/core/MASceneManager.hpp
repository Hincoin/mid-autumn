#ifndef MA_SCENE_MANAGER_HPP
#define MA_SCENE_MANAGER_HPP
#include "SceneManager.hpp"
namespace ma{
	template<typename Config>
	class MASceneManager:public SceneManager<MASceneManager<Config>,Config>
	{
		typedef typename Config::EventType EventType;
	public:
		bool postEventFromUser(const EventType& evt){return false;}
	};
}
#endif