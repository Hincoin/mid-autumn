#ifndef X11_DEVICE_TEST_HPP_INCLUDED
#define X11_DEVICE_TEST_HPP_INCLUDED
//this file is used for x11 device test
#include "MADeviceX11.hpp"
//#include "MADeviceX11Configure.hpp"
#include "MADeviceConfigure.hpp"
#include "CreateDevice.hpp"
#include "MAMath.hpp"

//#define TEXTURE_TEST
#define STENCIL_TEST
//#define SHADER_TEST
//
#ifdef STENCIL_TEST
#include "Stencil_buffer_test.hpp"
#endif

#ifdef TEXTURE_TEST
#include "TextureShader_Test.hpp"
#endif

#ifdef SHADER_TEST
#include "Shader_test.hpp"
#endif


struct x11_device_test_configure{
typedef TestDeviceConfigureX11 DeviceConfig;
typedef ma::MADeviceX11<DeviceConfig> DeviceType;
typedef DeviceType* DevicePtr;
typedef DeviceConfig::EventProcessorPtr EventProcessorPtr;
typedef DeviceConfig::DriverType DriverType;

};
namespace ma{
template<>
	struct CreateDevice<x11_device_test_configure>{
	    typedef x11_device_test_configure Config;
		Config::DevicePtr operator()(Config::DriverType driverType,
			scalar2i& windowSize,
			unsigned int bits, bool fullscreen,
			bool stencilbuffer, bool vsync,
			bool antiAlias,
			bool highPrecisionFPU,
			Config::EventProcessorPtr receiver,
			int externalWindow){
				typedef Config::DeviceType DeviceType;
				return new DeviceType(driverType,windowSize,bits,fullscreen,stencilbuffer,vsync,antiAlias,
			 highPrecisionFPU,
			receiver,
			externalWindow);
		}
	};

}


void inline device_test_func()
{
	using namespace ma;

	CreateDevice<x11_device_test_configure> device_creator;
	delete_ptr<x11_device_test_configure::DevicePtr> device_destroyer;
#ifdef _DEBUG
        scalar2i _800x600 = scalar2i(320,240);
#else
    scalar2i _800x600 = scalar2i(800,600);
#endif
	x11_device_test_configure::DevicePtr device_ =
	 device_creator(x11_device_test_configure::DriverType(0),
	 _800x600,(unsigned)32,
	 false,false,false,false,false,x11_device_test_configure::EventProcessorPtr(0), 0);

	//device_->setWindowCatption("Hello WOrld!   ---- mid-autumn demo");
	x11_device_test_configure::DeviceConfig::VideoDriverPtr
		driver_ptr = device_->getVideoDriver();
	//win32_device_test_configure::DeviceConfig::SceneManagerPtr
	//	smgr = device_->getSceneManager();
	//win32_device_test_configure::DeviceConfig::GUIManagerPtr
	//	guimgr = device_->getGUIEnvironment();
	driver_ptr->viewport(0,0,scalar2_op::width(_800x600),scalar2_op::height(_800x600));
	while (device_->execute())
	{
		if (device_->isWindowActive())
		{
			driver_ptr->beginScene(true,true,x11_device_test_configure::DeviceConfig::VideoDriver::Color(0));
			ma_test::test_fun(driver_ptr);
			driver_ptr->endScene(device_,0,0);
		}
		else
			device_->yield();
	}

	device_destroyer(device_);
}

#endif // X11_DEVICE_TEST_HPP_INCLUDED
