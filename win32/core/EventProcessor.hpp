#ifndef INCLUDE_EVENT_PROCESSOR_HPP
#define INCLUDE_EVENT_PROCESSOR_HPP

#include "CRTPInterfaceMacro.hpp"


namespace ma{
	


	template<typename Derived,typename ClassConfig>
	class EventProcessor{
		//ADD_CRTP_INTERFACE_TYPEDEF(EventType)
		typedef typename ClassConfig::EventType EventType;
	public:
		//! Called if an event happened.
		/** \return True if the event was processed. */
		bool onEvent(const EventType& event)
		{
			static_cast<Derived&>(*this).onEvent(event);
		}
	protected:
		~EventProcessor(){}
		EventProcessor(){}
	};
}


#endif