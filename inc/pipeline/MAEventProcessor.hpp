#ifndef MA_EVENT_PROCESSOR_HPP
#define MA_EVENT_PROCESSOR_HPP

#include "EventProcessor.hpp"
#include "KeyCodes.hpp"
#include "Logger.hpp"

namespace ma{
	template<typename Configure>
	class MAEventProcessor:public EventProcessor<MAEventProcessor<Configure>, Configure >{
		typedef typename Configure::EventType EventType;
	public:
		//! Called if an event happened.
		/** \return True if the event was processed. */
		bool onEvent(const EventType& event)
		{
			return false;
		}
	};
}
#endif