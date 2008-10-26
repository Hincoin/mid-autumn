#ifndef MA_EVENT_PROCESSOR_IMPL_HPP
#define MA_EVENT_PROCESSOR_IMPL_HPP

#include "EventProcessor.hpp"
namespace ma{
	template<typename Configure>
	class MAEventProcessor:public EventProcessor<MAEventProcessor<Configure> >{
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