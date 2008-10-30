#ifndef INCLUDE_MADEVICE_HPP
#define INCLUDE_MADEVICE_HPP

#include "MADeviceBase.hpp"

namespace ma{
	template<typename Derived,typename Configure>
	class MADevice:public MADeviceBase<MADevice<Derived,Configure>, Configure >
	{
	private:
		Derived& derived(){return static_cast<Derived&>(*this);}
		const Derived& derived()const{return static_cast<Derived&>(*this);}
	public:
		typedef typename Configure::Printer Printer;
		typedef typename Configure::VideoDriver VideoDriver;
		typedef typename Configure::FileSystem FileSystem;
		typedef typename Configure::GUIManager GUIManager;
		typedef typename Configure::SceneManager SceneManager;
		typedef typename Configure::Logger Logger;
		typedef typename Configure::VideoMode VideoMode;
		typedef typename Configure::OSOperator OSOperator;
		typedef typename Configure::Timer Timer;
		typedef typename Configure::EventProcessor EventProcessor;

		//typedef typename Configure::EventType     EventType;

		typedef typename Configure::VideoDriverPtr VideoDriverPtr;
		typedef typename Configure::FileSystemPtr FileSystemPtr;
		typedef typename Configure::GUIManagerPtr GUIManagerPtr;
		typedef typename Configure::SceneManagerPtr SceneManagerPtr;
		typedef typename Configure::LoggerPtr LoggerPtr;
		typedef typename Configure::VideoModePtr VideoModePtr;
		typedef typename Configure::OSOperatorPtr OSOperatorPtr;
		typedef typename Configure::TimerPtr TimerPtr;
		typedef typename Configure::EventProcessorPtr EventProcessorPtr;

	protected:
		explicit MADevice(EventProcessorPtr evt_processor);
		~MADevice();
		void createGUIAndScene();
	public:
		bool execute(){derived().execute();}
		void yield(){derived().yield();}
		void sleep(std::size_t timeMs, bool is_pauseTimer){derived().sleep(timeMs,is_pauseTimer);}

		VideoDriverPtr getVideoDriver(){return VideoDriver_;}
		FileSystemPtr getFileSystem(){return FileSystem_;}
		GUIManagerPtr getGUIEnvironment(){return GUIEnvironment_;}
		SceneManagerPtr getSceneManager(){return SceneManager_;}

		//! \return Returns a pointer to the mouse cursor control interface.
		//virtual gui::ICursorControl* getCursorControl();

		VideoModePtr getVideoModes(){return VideoModeList_;}
		TimerPtr getTimer(){return Timer_;}
		static const char* getVersion(){return ma_version;}
		LoggerPtr getLogger(){return Logger_;}
		OSOperatorPtr getOSOperator(){return OSOperator_;}
		EventProcessorPtr getEventReceiver(){return UserReceiver_;}

		template<typename EventType>
		void postEventFromUser(const EventType& event);

		void setEventReceiver(EventProcessorPtr receiver);
		void setInputReceivingSceneManager(SceneManagerPtr sceneManager);

	protected:

		VideoDriverPtr VideoDriver_;
		GUIManagerPtr GUIEnvironment_;
		SceneManagerPtr SceneManager_;
		TimerPtr Timer_;
		//CursorControlPtr CursorControl_;
		VideoModePtr VideoModeList_;
		EventProcessorPtr UserReceiver_;
		LoggerPtr Logger_;
		OSOperatorPtr OSOperator_;
		FileSystemPtr FileSystem_;
		SceneManagerPtr InputReceivingSceneManager_;
	};
}
#endif

#include "MADeviceImpl.hpp"