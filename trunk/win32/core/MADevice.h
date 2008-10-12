#ifndef MADEVICE_H
#define MADEVICE_H

#include "MADeviceBase.h"

namespace ma{
	template<typename Derived>
	class MADevice:public MADeviceBase<MADevice<Derived> >
	{
	private:
		Derived& derived(){return static_cast<Derived&>(*this);}
		const Derived& derived()const{return static_cast<Derived&>(*this);}
	public:
		typedef typename Derived::Configure Configure;

		typedef typename Derived::Printer Printer;
		typedef typename Derived::VideoDriver VideoDriver;
		typedef typename Derived::FileSystem FileSystem;
		typedef typename Derived::GUIManager GUIManager;
		typedef typename Derived::SceneManager SceneManager;
		typedef typename Derived::Logger Logger;
		typedef typename Derived::VideoMode VideoMode;
		typedef typename Derived::OSOperator OSOperator;
		typedef typename Derived::Timer Timer;
		typedef typename Derived::EventProcessor EventProcessor;

		typedef typename Derived::EventType     EventType;

		typedef typename Derived::VideoDriverPtr VideoDriverPtr;
		typedef typename Derived::FileSystemPtr FileSystemPtr;
		typedef typename Derived::GUIManagerPtr GUIManagerPtr;
		typedef typename Derived::SceneManagerPtr SceneManagerPtr;
		typedef typename Derived::LoggerPtr LoggerPtr;
		typedef typename Derived::VideoModePtr VideoModePtr;
		typedef typename Derived::OSOperatorPtr OSOperatorPtr;
		typedef typename Derived::TimerPtr TimerPtr;
		typedef typename Derived::EventProcessorPtr EventProcessorPtr;

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

		void postEventFromUser(const EventType& event);
		void setEventReceiver(EventProcessorPtr receiver);
		void setInputReceivingSceneManager(SceneManagerPtr sceneManager);

	private:

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

#include "MADeviceImpl.h"