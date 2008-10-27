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


namespace ma{
	enum DriverType{};

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
	MA_SPECIALIZE_TYPEDEF_TRAITS_TYPE(ReadFile,ma::MAFileSystemWin32<ma::empty_config_file_system>,empty_config_file_system)
		MA_SPECIALIZE_TYPEDEF_TRAITS_TYPE(WriteFile,ma::MAFileSystemWin32<ma::empty_config_file_system>,empty_config_file_system)
		MA_SPECIALIZE_TYPEDEF_TRAITS_TYPE(FileList,ma::MAFileSystemWin32<ma::empty_config_file_system>,empty_config_file_system)
		MA_SPECIALIZE_TYPEDEF_TRAITS_TYPE(ReadFilePtr,ma::MAFileSystemWin32<empty_config_file_system>,empty_config_file_system)
		MA_SPECIALIZE_TYPEDEF_TRAITS_TYPE(WriteFilePtr,ma::MAFileSystemWin32<empty_config_file_system>,empty_config_file_system)
		MA_SPECIALIZE_TYPEDEF_TRAITS_TYPE(FileListPtr,ma::MAFileSystemWin32<empty_config_file_system>,empty_config_file_system)

}
struct TestDeviceConfigureWin32{



	//make it compile
	typedef ma::test_empty ImagePresenter;
	typedef ma::test_empty* ImagePtr;

	typedef ma::Printer<ma::default_printer_config<ma::MALogger> > Printer;
	typedef ma::MAVideoDriverSoftWare<ma::default_printer_config<ma::MALogger> > VideoDriver;
	typedef ma::MAFileSystemWin32<ma::empty_config_file_system> FileSystem;
	typedef ma::MAGUIManager GUIManager;
	typedef ma::MASceneManager SceneManager;
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
};

#endif