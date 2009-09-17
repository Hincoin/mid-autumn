#ifndef MA_GUI_MANAGER_HPP
#define MA_GUI_MANAGER_HPP

#include "GUIManager.hpp"
namespace ma{
	template<typename Config>
	class MAGUIManager:public GUIManager<MAGUIManager<Config>,Config>{
		typedef typename Config::EventType EventType;
	public:
		bool postEventFromUser(const EventType& evt){ return false;}
	};
}
#endif