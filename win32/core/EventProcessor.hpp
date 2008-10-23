#ifndef MA_EVENT_PROCESSOR_HPP
#define MA_EVENT_PROCESSOR_HPP

#include "CRTPInterfaceMacro.hpp"


namespace ma{
	
	MA_DECLARE_TYPEDEF_TRAITS_TYPE(EventType);

	template<typename Derived>
	class EventProcessor{
		ADD_CRTP_INTERFACE_TYPEDEF(EventType)
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