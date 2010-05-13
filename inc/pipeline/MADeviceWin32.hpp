#ifndef MADEVICEWIN32_H
#define MADEVICEWIN32_H
#include "MAConfig.hpp"

#ifdef MA_WINDOWS_

#include "MADevice.hpp"
#include "Vector.hpp"
#include <string>
#include <boost/bimap.hpp>
#include "SpaceSegment.hpp"

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>


namespace ma
{
	template<typename Configure_T>
	class MADeviceWin32:public MADevice<MADeviceWin32<Configure_T>, Configure_T >,  Configure_T::ImagePresenter
	{
	public:
		typedef Configure_T Configure;
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

		typedef typename Configure::VideoDriverPtr VideoDriverPtr;
		typedef typename Configure::FileSystemPtr FileSystemPtr;
		typedef typename Configure::GUIManagerPtr GUIManagerPtr;
		typedef typename Configure::SceneManagerPtr SceneManagerPtr;
		typedef typename Configure::LoggerPtr LoggerPtr;
		typedef typename Configure::VideoModePtr VideoModePtr;
		typedef typename Configure::OSOperatorPtr OSOperatorPtr;
		typedef typename Configure::TimerPtr TimerPtr;
		typedef typename Configure::EventProcessorPtr EventProcessorPtr;

		typedef typename Configure::DriverType DriverType;

		typedef MADevice<MADeviceWin32<Configure_T>, Configure_T > DeviceBase;
	public:
		MADeviceWin32(DriverType driverType,
			scalar2i& windowSize,
			unsigned int bits, bool fullscreen,
			bool stencilbuffer, bool vsync,
			bool antiAlias,
			bool highPrecisionFPU,
			EventProcessorPtr receiver,
			HWND externalWindow);
		~MADeviceWin32();

		bool execute();
		void yield();
		void sleep(std::size_t timeMs, bool is_pauseTimer);

		//! sets the caption of the window
		void setWindowCaption(const wchar_t* text);

		//! returns if window is active. if not, nothing need to be drawn
		bool isWindowActive() const;

		//! presents a surface in the client area
		template<typename ImagePtr>
		void present(ImagePtr surface, int windowId, recti* src );

		//! notifies the device that it should close itself
		void closeDevice();

		//! \return Returns a pointer to a list with all video modes
		//! supported by the gfx adapter.
		VideoModePtr getVideoModeList();

		//! Notifies the device, that it has been resized
		void OnResized();

		//! Sets if the window should be resizeable in windowed mode.
		void setResizeAble(bool resize);
	private:
		//! create the driver
		void createDriver(DriverType driverType,
			const scalar2i& windowSize, unsigned int bits, bool fullscreen,
			bool stencilbuffer, bool vsync, bool antiAlias, bool highPrecisionFPU);

		//! switches to fullscreen
		bool switchToFullScreen(int width, int height, int bits);

		std::string getWindowsVersion(/*const std::string& version*/){return "";}

		void resizeIfNecessary();

		HWND HWnd;

		bool ChangedToFullScreen;
		bool FullScreen;
		bool IsNonNTWindows;
		bool Resized;
		bool ExternalWindow;

		typedef boost::bimap<HWND,MADeviceWin32<Configure>* > EnvironmentMap;
		static  EnvironmentMap environment_map_;

	private:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		static MADeviceWin32<Configure>* getDeviceFromHWnd(HWND hwnd);
	};
	template<typename Configure>
	boost::bimap<HWND,MADeviceWin32<Configure>*>
		MADeviceWin32<Configure>::environment_map_;


	template<int N>
	inline wchar_t* char2wchar(const char a[N],wchar_t b[N/2 + 1])
	{
		size_t i = 0;
		for (;i < N - 1 ; i+=2)
		{
			b[i>>1] = wchar_t(a[i]) << 8 | a[i +1];
		}
		if(i == N-1)
			b[(N>>1)] = a[i];
		else
			b[(N>>1)] = 0;
		return b;
	}
}
#endif

#endif

#ifdef MA_WINDOWS_
#include "MADeviceWin32Impl.hpp"
#endif

