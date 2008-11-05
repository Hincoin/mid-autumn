

#ifndef WIN32_DEVICE_TEST_HPP
#define WIN32_DEVICE_TEST_HPP

#include "MADeviceWin32.hpp"

#include "MADeviceConfigure.hpp"
#include "CreateDevice.hpp"
#include "PtrTraits.hpp"

#include "MAMath.hpp"

struct win32_device_test_configure{
	//MADeviceWin32(DriverType driverType,
	//	const rangei& windowSize,
	//	unsigned int bits, bool fullscreen,
	//	bool stencilbuffer, bool vsync,
	//	bool antiAlias,
	//	bool highPrecisionFPU,
	//	EventProcessorPtr receiver,
	//	HWND externalWindow);
	typedef TestDeviceConfigureWin32 DeviceConfig;
	typedef ma::MADeviceWin32<TestDeviceConfigureWin32> DeviceType;

	typedef DeviceType* DevicePtr;
	typedef DeviceConfig::EventProcessorPtr EventProcessorPtr;
	typedef DeviceConfig::DriverType DriverType;

};

namespace ma{

	template<>
	struct CreateDevice<win32_device_test_configure>{
		win32_device_test_configure::DevicePtr operator()(win32_device_test_configure::DriverType driverType,
			scalar2i& windowSize,
			unsigned int bits, bool fullscreen,
			bool stencilbuffer, bool vsync,
			bool antiAlias,
			bool highPrecisionFPU,
			win32_device_test_configure::EventProcessorPtr receiver,
			HWND externalWindow){return new win32_device_test_configure::DeviceType(driverType,windowSize,bits,fullscreen,stencilbuffer,vsync,antiAlias,highPrecisionFPU,receiver,externalWindow);}
	};
}

#include "MAVertex.hpp"

typedef ma::MAVertex<ma::vector2i> vertex2i;
inline void device_test_func(){
	struct tmp_t{
		bool operator == (const tmp_t&) const{return false;}
	};
	assert(ma::equal(1,1));
	assert(ma::equal(1.f,1));
	assert(ma::equal(2.000,2));
	assert(ma::equal(tmp_t(),tmp_t()));

	vertex2i a;
	a.x();

	using namespace ma;

	CreateDevice<win32_device_test_configure> device_creator;
	delete_ptr<win32_device_test_configure::DevicePtr> device_destroyer;

	win32_device_test_configure::DevicePtr device_ = device_creator(win32_device_test_configure::DriverType(0),scalar2i(800,600),32,false,false,false,0,false,0,0);
	
	//device_->setWindowCatption("Hello WOrld!   ---- mid-autumn demo");
	win32_device_test_configure::DeviceConfig::VideoDriverPtr
		driver_ptr = device_->getVideoDriver();
	win32_device_test_configure::DeviceConfig::SceneManagerPtr
		smgr = device_->getSceneManager();
	win32_device_test_configure::DeviceConfig::GUIManagerPtr
		guimgr = device_->getGUIEnvironment();

	while (device_->execute())
	{
		driver_ptr->beginScene(true,true,win32_device_test_configure::DeviceConfig::VideoDriver::Color(0));
	
		driver_ptr->endScene(device_,0,0);
	}

	device_destroyer(device_);
	

}


#endif