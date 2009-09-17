#ifndef INCLUDE_EVENT_TYPE_HPP
#define INCLUDE_EVENT_TYPE_HPP

namespace ma{
	template<typename ClassConfig >
	struct EventType{
		typedef typename ClassConfig::GUIEvent GUIEvent;
		typedef typename ClassConfig::MouseInput MouseInput;
		typedef typename ClassConfig::KeyInput KeyInput;
		typedef typename ClassConfig::LogEvent LogEvent;
		typedef typename ClassConfig::UserEvent UserEvent;

		typename ClassConfig::EVENT_TYPE event_type;
		union{
			GUIEvent gui_event;
			MouseInput mouse_event;
			KeyInput key_event;
			LogEvent log_event;
			UserEvent user_event;
		};
	};
}


#endif