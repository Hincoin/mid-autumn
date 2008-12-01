#ifndef INCLUDE_GUIMANAGER_HPP
#define INCLUDE_GUIMANAGER_HPP

#include "NullType.hpp"
namespace ma{
	template<typename Derived,typename ClassConfig>
	class GUIManager{
		Derived& derived(){return static_cast<Derived&>(*this);}
		const Derived& derived()const{return static_cast<const Derived&>(*this);}

		typedef typename ClassConfig::EventType EventType;
	public:
		bool postEventFromUser(const EventType& evt){return derived().postEventFromUser(evt);}
	};
}

#endif
