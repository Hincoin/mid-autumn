#ifndef MADEVICECONFIGURE_HPP
#define MADEVICECONFIGURE_HPP

//#include ""
struct TestDeviceConfigureWin32{
	typedef MAPrinter Printer;
	typedef MAVideoDriver VideoDriver;
	typedef MAFileSystem FileSystem;
	typedef MAGUIManager GUIManager;
	typedef MASceneManager SceneManager;
	typedef MALogger Logger;
	typedef MAVideoMode VideoMode;
	typedef MAOSOperator OSOperator;
	typedef MATimer Timer;
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