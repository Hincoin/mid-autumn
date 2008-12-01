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
#include "MAImage.hpp"


#include "CreateDevice.hpp"

#include "PtrTraits.hpp"

#include "AddPointer.hpp"

#include "MAColor.hpp"

#include "MAGeometryRenderer.hpp"
#include "MAAffineRasterizer.hpp"

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


	struct default_image_config{
		typedef Colori Color;//4 ints
	};

	struct default_video_driver_software_config{
		typedef default_image_config::Color Color;
		typedef ma::MAFileSystemWin32<ma::empty_config_file_system> FileSystem;
		typedef FileSystem* FileSystemPtr;
		typedef ma::MAImage<default_image_config> Image;
		typedef ma::MADepthBuffer DepthBuffer;
		typedef add_ptr<Image>::type ImagePtr;
		typedef add_ptr<DepthBuffer>::type DepthBufferPtr;

		typedef test_empty Texture;
		typedef add_ptr<Texture>::type TexturePtr;
		typedef MAAffineRasterizer Rasterizer;
		typedef MAGeometryRenderer<MAAffineRasterizer> GeometryRenderer;

		typedef boost::shared_ptr<MAAffineRasterizer> RasterizerPtr;
		//typedef MAGeometryRenderer<MAAffineRasterizer>* GeometryRendererPtr;
		typedef boost::shared_ptr<MAGeometryRenderer<MAAffineRasterizer> > GeometryRendererPtr;
	};
	//default driver creator traits
	template<>
	struct CreateDriver<MAVideoDriverSoftWare<default_video_driver_software_config> >{
		template<typename Config> friend  class MADeviceWin32;

		typedef MAVideoDriverSoftWare<default_video_driver_software_config>::FileSystemPtr FileSystemPtr;
	private:
		MAVideoDriverSoftWare<default_video_driver_software_config>*
			operator()(const scalar2i& windowSize,bool fullscreen, FileSystemPtr fs)const{
				return new MAVideoDriverSoftWare<default_video_driver_software_config>(windowSize, fullscreen, fs);
		}
	};
}
struct TestDeviceConfigureWin32{
	enum DriverType{};
	//make it compile
	typedef ma::test_empty ImagePresenter;
	typedef ma::test_empty* ImagePtr;

	typedef ma::Printer<ma::default_printer_config<ma::MALogger> > Printer;
	typedef ma::MAVideoDriverSoftWare<ma::default_video_driver_software_config > VideoDriver;
	typedef ma::MAFileSystemWin32<ma::empty_config_file_system> FileSystem;
	typedef ma::MAGUIManager<ma::event_processor_config> GUIManager;
	typedef ma::MASceneManager<ma::event_processor_config> SceneManager;
	typedef ma::MALogger Logger;
	typedef ma::MAVideoModesWin32 VideoMode;
	typedef ma::MAOSOperatorWin32 OSOperator;
	typedef ma::MATimerWin32 Timer;
	typedef ma::MAEventProcessor<ma::event_processor_config> EventProcessor;

	typedef ma::MAEventType     EventType;

	typedef VideoDriver* VideoDriverPtr;
	typedef FileSystem* FileSystemPtr;
	typedef GUIManager* GUIManagerPtr;
	typedef SceneManager* SceneManagerPtr;
	typedef Logger* LoggerPtr;
	typedef VideoMode* VideoModePtr;
	typedef OSOperator* OSOperatorPtr;
	typedef Timer* TimerPtr;
	typedef EventProcessor* EventProcessorPtr;

	typedef ma::CreateDriver<ma::MAVideoDriverSoftWare<ma::default_video_driver_software_config> > DriverCreator;

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
