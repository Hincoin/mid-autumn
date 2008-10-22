#ifndef MA_EVENT_PROCESSOR_HPP
#define MA_EVENT_PROCESSOR_HPP


namespace ma{
	template<typename Derived>
	class EventProcessor{

		typedef Derived::EventType EventType;
	public:
		//! Called if an event happened.
		/** \return True if the event was processed. */
		bool onEvent(const EventType& event)
		{
			static_cast<Derived&>(*this).OnEvent();
		}
	protected:
		~EventProcessor(){}
		EventProcessor(){}
	};
}


#endif