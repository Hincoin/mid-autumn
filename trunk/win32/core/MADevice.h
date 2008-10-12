#ifndef MADEVICE_H
#define MADEVICE_H

#include "MADeviceBase.h"

namespace ma{
	template<typename Configure>
	class MADevice:public MADeviceBase<MADevice<Configure> >
	{
	public:
		typedef typename Configure::VideoDriver VideoDriver;
		typedef typename Configure::FileSystem FileSystem;
		typedef typename Configure::GUIManager GUIManager;
		typedef typename Configure::SceneManager SceneManager;
		typedef typename Configure::Logger Logger;
		typedef typename Configure::VideoMode VideoMode;
		typedef typename Configure::OSOperator OSOperator;
		typedef typename Configure::Timer Timer;
		typedef typename Configure::EventProcessor EventProcessor;

		typedef typename Configure::EventType     EventType;

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
		MADevice(){}
		~MADevice(){
			Configure::delete_ptr(VideoDriver_);
			Configure::delete_ptr(GUIEnvironment_);
			Configure::delete_ptr(SceneManager_);
			Configure::delete_ptr(Timer_);
			Configure::delete_ptr(VideoModeList_);
			Configure::delete_ptr(UserReceiver_);
			Configure::delete_ptr(Logger_);
			Configure::delete_ptr(OSOperator_);
			Configure::delete_ptr(FileSystem_);
			Configure::delete_ptr(InputReceivingSceneManager_);
		}
	public:
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