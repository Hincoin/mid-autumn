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

struct TestDeviceConfigureWin32{
	typedef MAPrinter Printer;
	typedef MAVideoDriver VideoDriver;
	typedef MAFileSystemWin32 FileSystem;
	typedef MAGUIManager GUIManager;
	typedef MASceneManager SceneManager;
	typedef MALogger Logger;
	typedef MAVideoModesWin32 VideoMode;
	typedef MAOSOperatorWin32 OSOperator;
	typedef MATimerWin32 Timer;
	typedef MAEventProcessor EventProcessor;

	typedef MAEventType     EventType;

	typedef MAVideoDriverPtr VideoDriverPtr;
	typedef MAFileSystemPtr FileSystemPtr;
	typedef MAGUIManagerPtr GUIManagerPtr;
	typedef MASceneManagerPtr SceneManagerPtr;
	typedef MALoggerPtr LoggerPtr;
	typedef MAVideoModePtr VideoModePtr;
	typedef MAOSOperatorPtr OSOperatorPtr;
	typedef MATimerPtr TimerPtr;
	typedef MAEventProcessorPtr EventProcessorPtr;
};

#endif