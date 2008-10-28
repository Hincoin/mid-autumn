#ifndef MADEVICECONFIGURE_HPP
#define MADEVICECONFIGURE_HPP

//implementations
#include "MAEventProcessor.hpp"
#include "MAFileSystemWin32.hpp"
#include "MALogger.hpp"
#include "MAOSOperatorWin32.hpp"
#include "MASceneManager.hpp"
#include "MATimerWin32.hpp"
#include "MAVideoModesWin32.hpp"
#include "MAPrinter.hpp"

#include "MAVideoDriverSoftWare.hpp"
#include "MAGUIManager.hpp"
#include "MAEventType.hpp"

#include "CreateDevice.hpp"

#include "PtrTraits.hpp"
namespace ma{
	

	struct test_empty{};
	struct empty_config_file_system{
		typedef test_empty ReadFile;
		typedef test_empty WriteFile;
		typedef test_empty FileList;
		typedef ReadFile* ReadFilePtr;
		typedef WriteFile* WriteFilePtr;
		typedef FileList* FileListPtr;

	};
	struct event_processor_config{
		typedef ma::MAEventType EventType;
	};
}
struct TestDeviceConfigureWin32{



	enum DriverType{};
	//make it compile
	typedef ma::test_empty ImagePresenter;
	typedef ma::test_empty* ImagePtr;

	typedef ma::Printer<ma::default_printer_config<ma::MALogger> > Printer;
	typedef ma::MAVideoDriverSoftWare<ma::default_printer_config<ma::MALogger> > VideoDriver;
	typedef ma::MAFileSystemWin32<ma::empty_config_file_system> FileSystem;
	typedef ma::MAGUIManager<ma::event_processor_config> GUIManager;
	typedef ma::MASceneManager<ma::event_processor_config> SceneManager;
	typedef ma::MALogger Logger;
	typedef ma::MAVideoModesWin32 VideoMode;
	typedef ma::MAOSOperatorWin32 OSOperator;
	typedef ma::MATimerWin32 Timer;
	typedef ma::MAEventProcessor<ma::event_processor_config> EventProcessor;

	typedef ma::MAEventType     EventType;

	typedef Printer* VideoDriverPtr;
	typedef FileSystem* FileSystemPtr;
	typedef GUIManager* GUIManagerPtr;
	typedef SceneManager* SceneManagerPtr;
	typedef Logger* LoggerPtr;
	typedef VideoMode* VideoModePtr;
	typedef OSOperator* OSOperatorPtr;
	typedef Timer* TimerPtr;
	typedef EventProcessor* EventProcessorPtr;

	template<typename PtrType>
	static inline void delete_ptr(PtrType p)
	{
		ma::delete_ptr<PtrType>()(p);
	}
	template<typename PtrType>
	static inline void empty_ptr(PtrType ptr)
	{
		ma::empty_ptr<PtrType>()(ptr);
	}

	static inline GUIManagerPtr createGUIEnvironment(FileSystemPtr fs_ptr,VideoDriverPtr video_driver_ptr,OSOperatorPtr os_ptr)
	{
		return GUIManagerPtr(new GUIManager());
	}
	static inline SceneManagerPtr createSceneManager(VideoDriverPtr video_driver_ptr,FileSystemPtr fs_ptr,GUIManagerPtr gui_ptr)
	{
		return SceneManagerPtr(new SceneManager());
	}

};

#endif