#ifndef MADEVICEWIN32IMPL_HPP
#define MADEVICEWIN32IMPL_HPP

#include "MADevice.hpp"
#include "MADeviceWin32.hpp"
#include "MAEventType.hpp"
#include <winuser.h>



namespace ma{


	template<typename Configure>
	MADeviceWin32<Configure>* MADeviceWin32<Configure>::getDeviceFromHWnd(HWND hwnd)
	{
		typedef typename EnvironmentMap::left_iterator EnvMapLeftIterator;
		EnvMapLeftIterator env_it = environment_map_.left.find(hwnd);
		return env_it == environment_map_.left.end() ? 0 : env_it->second ;
	}
		template<typename Configure>
		LRESULT CALLBACK MADeviceWin32<Configure>::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif
			MADeviceWin32<Configure>* dev = 0;

			typedef typename Configure::EventType EventType;
			EventType event;

			typedef typename EnvironmentMap::left_iterator EnvMapLeftIterator;
			//SEnvMapper* envm = 0;

			BYTE allKeys[256];

			static int ClickCount=0;
			if (GetCapture() != hWnd && ClickCount > 0)
				ClickCount = 0;

			switch (message)
			{
			case WM_PAINT:
				{
					PAINTSTRUCT ps;
					BeginPaint(hWnd, &ps);
					EndPaint(hWnd, &ps);
				}
				return 0;

			case WM_ERASEBKGND:
				return 0;

			case WM_SETCURSOR:
				{
					////envm = getEnvMapperFromHWnd(hWnd);
					EnvMapLeftIterator env_it = environment_map_.left.find(hWnd);
					if (env_it != environment_map_.left.end()
						/*&& !env_it->second->getWin32CursorControl()->isVisible()*/)
					{
						SetCursor(NULL);
						return 0;
					}
				}

				break;

			case WM_MOUSEWHEEL:
				//event.EventType = irr::EET_MOUSE_INPUT_EVENT;
				//event.MouseInput.Wheel = ((irr::f32)((short)HIWORD(wParam))) / (irr::f32)WHEEL_DELTA;
				//event.MouseInput.Event = irr::EMIE_MOUSE_WHEEL;
				event.event_type = EET_MOUSE_INPUT_EVENT;
				event.mouse_event.Wheel = float((short)HIWORD(wParam))/ (float)WHEEL_DELTA;
				event.mouse_event.Wheel = EMIE_MOUSE_WHEEL;

				POINT p; // fixed by jox
				p.x = 0; p.y = 0;
				ClientToScreen(hWnd, &p);
				event.mouse_event.x = LOWORD(lParam) - p.x;
				event.mouse_event.y = HIWORD(lParam) - p.y;

				dev = getDeviceFromHWnd(hWnd);
				if (dev)
					dev->postEventFromUser(event);
				break;

			case WM_LBUTTONDOWN:
				ClickCount++;
				SetCapture(hWnd);

				event.event_type = EET_MOUSE_INPUT_EVENT;
				event.mouse_event.Event = EMIE_LMOUSE_DOWN;
				event.mouse_event.x = (short)LOWORD(lParam);
				event.mouse_event.y = (short)HIWORD(lParam);
				dev = getDeviceFromHWnd(hWnd);
				if (dev)
					dev->postEventFromUser(event);

				return 0;

			case WM_LBUTTONUP:
				ClickCount--;
				if (ClickCount<1)
				{
					ClickCount=0;
					ReleaseCapture();
				}

				event.event_type =  EET_MOUSE_INPUT_EVENT;
				event.mouse_event.Event = EMIE_LMOUSE_UP;
				event.mouse_event.x = (short)LOWORD(lParam);
				event.mouse_event.y= (short)HIWORD(lParam);

				dev = getDeviceFromHWnd(hWnd);
				if (dev)
					dev->postEventFromUser(event);

				return 0;

			case WM_RBUTTONDOWN:
				ClickCount++;
				SetCapture(hWnd);
				event.event_type = EET_MOUSE_INPUT_EVENT;
				event.mouse_event.Event = EMIE_RMOUSE_DOWN;
				event.mouse_event.x = (short)LOWORD(lParam);
				event.mouse_event.y = (short)HIWORD(lParam);

				dev = getDeviceFromHWnd(hWnd);
				if (dev)
					dev->postEventFromUser(event);
				return 0;

			case WM_RBUTTONUP:
				ClickCount--;
				if (ClickCount<1)
				{
					ClickCount=0;
					ReleaseCapture();
				}
				event.event_type =  EET_MOUSE_INPUT_EVENT;
				event.mouse_event.Event =  EMIE_RMOUSE_UP;
				event.mouse_event.x = (short)LOWORD(lParam);
				event.mouse_event.y = (short)HIWORD(lParam);

				dev = getDeviceFromHWnd(hWnd);
				if (dev)
					dev->postEventFromUser(event);
				return 0;

			case WM_MBUTTONDOWN:
				ClickCount++;
				SetCapture(hWnd);
				event.event_type =  EET_MOUSE_INPUT_EVENT;
				event.mouse_event.Event =  EMIE_MMOUSE_DOWN;
				event.mouse_event.x = (short)LOWORD(lParam);
				event.mouse_event.y = (short)HIWORD(lParam);

				dev = getDeviceFromHWnd(hWnd);
				if (dev)
					dev->postEventFromUser(event);
				return 0;

			case WM_MBUTTONUP:
				ClickCount--;
				if (ClickCount<1)
				{
					ClickCount=0;
					ReleaseCapture();
				}
				event.event_type =  EET_MOUSE_INPUT_EVENT;
				event.mouse_event.Event =  EMIE_MMOUSE_UP;
				event.mouse_event.x = (short)LOWORD(lParam);
				event.mouse_event.y = (short)HIWORD(lParam);
				dev = getDeviceFromHWnd(hWnd);
				if (dev)
					dev->postEventFromUser(event);
				return 0;

			case WM_MOUSEMOVE:
				event.event_type=  EET_MOUSE_INPUT_EVENT;
				event.mouse_event.Event =  EMIE_MOUSE_MOVED;
				event.mouse_event.x = (short)LOWORD(lParam);
				event.mouse_event.y = (short)HIWORD(lParam);
				dev = getDeviceFromHWnd(hWnd);

				if (dev)
					dev->postEventFromUser(event);

				return 0;

			case WM_KEYDOWN:
			case WM_KEYUP:
				{
					event.event_type = EET_KEY_INPUT_EVENT;
					event.key_event.Key = (EKEY_CODE)wParam;
					event.key_event.PressedDown = (message==WM_KEYDOWN);
				    dev = getDeviceFromHWnd(hWnd);

					WORD KeyAsc=0;
					GetKeyboardState(allKeys);
					ToAscii((UINT)wParam,(UINT)lParam,allKeys,&KeyAsc,0);

					event.key_event.Shift = ((allKeys[VK_SHIFT] & 0x80)!=0);
					event.key_event.Control = ((allKeys[VK_CONTROL] & 0x80)!=0);
					event.key_event.Char = (KeyAsc & 0x00ff); //KeyAsc >= 0 ? KeyAsc : 0;

					if (dev)
						dev->postEventFromUser(event);

					return 0;
				}

			case WM_SIZE:
				{
					// resize
					dev = getDeviceFromHWnd(hWnd);
					if (dev)
						dev->OnResized();
				}
				return 0;

			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;

			case WM_SYSCOMMAND:
				// prevent screensaver or monitor powersave mode from starting
				if ((wParam & 0xFFF0) == SC_SCREENSAVE ||
					(wParam & 0xFFF0) == SC_MONITORPOWER)
					return 0;
				break;
			}

			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	template<typename Configure>
	MADeviceWin32<Configure>::MADeviceWin32
		(DriverType driverType, scalar2i& windowSize,
		unsigned int bits, bool fullscreen, bool stencilbuffer,
		bool vsync, bool antiAlias, bool highPrecisionFPU,
		EventProcessorPtr receiver, HWND externalWindow)
		:
	MADevice<MADeviceWin32<Configure>,Configure >(receiver),
		HWnd(0),ChangedToFullScreen(false),
		FullScreen(fullscreen),IsNonNTWindows(false),Resized(false),
		ExternalWindow(false)
	{
		std::string winversion(getWindowsVersion());
		DeviceBase::OSOperator_ = new OSOperator(/*winversion*/);
		//Printer::log(winversion.c_str(), ELL_INFORMATION);

		// create window

		HINSTANCE hInstance = GetModuleHandle(0);

#ifdef _DEBUG
		//setDebugName("CIrrDeviceWin32");
#endif

		// create the window, only if we do not use the null device
		if (/*driverType != video::EDT_NULL && */externalWindow==0)
		{
#ifdef UNICODE
			const char c_name[]  = "CIrrDeviceWin32";
			wchar_t ClassName[sizeof(c_name)/2 + 1]={0};
			char2wchar<sizeof(c_name)>(c_name,ClassName);
#else
			const char* ClassName = "CIrrDeviceWin32";
#endif


			// Register Class
			WNDCLASSEX wcex;
			wcex.cbSize		= sizeof(WNDCLASSEX);
			wcex.style		= CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc	= (WNDPROC)(MADeviceWin32<Configure>::WndProc);
			wcex.cbClsExtra		= 0;
			wcex.cbWndExtra		= 0;
			wcex.hInstance		= hInstance;
			wcex.hIcon		= NULL;
			wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
			wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
			wcex.lpszMenuName	= 0;
			wcex.lpszClassName	= ClassName;
			wcex.hIconSm		= 0;

			// if there is an icon, load it
			wcex.hIcon = NULL;//(HICON)LoadImage(hInstance, 0, IMAGE_ICON, 0,0, LR_LOADFROMFILE);

			RegisterClassEx(&wcex);

			// calculate client size

			RECT clientSize;
			clientSize.top = 0;
			clientSize.left = 0;
			clientSize.right = windowSize[0];
			clientSize.bottom = windowSize[1];

			DWORD style = WS_POPUP;

			if (!fullscreen)
				style = WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS
				| WS_MAXIMIZEBOX | WS_MINIMIZEBOX |  WS_OVERLAPPEDWINDOW;

			AdjustWindowRect(&clientSize, style, FALSE);

			int realWidth = clientSize.right - clientSize.left;
			int realHeight = clientSize.bottom - clientSize.top;

			int windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
			int windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

			if (fullscreen)
			{
				windowLeft = 0;
				windowTop = 0;
			}

			// create window

			HWnd = CreateWindow( ClassName, 0, style, windowLeft, windowTop,
				realWidth, realHeight,	NULL, NULL, hInstance, NULL);

			ShowWindow(HWnd , SW_SHOW);
			UpdateWindow(HWnd);

			// fix ugly ATI driver bugs. Thanks to ariaci
			MoveWindow(HWnd, windowLeft, windowTop, realWidth, realHeight, TRUE);
		}

		// attach external window
		if (externalWindow)
		{
			HWnd = externalWindow;
			RECT r;
			GetWindowRect(HWnd, &r);
			windowSize[0] = r.right - r.left;
			windowSize[1] = r.bottom - r.top;
			fullscreen = false;
			ExternalWindow = true;
		}

		// create cursor control

		//Win32CursorControl = new CCursorControl(windowSize, HWnd, fullscreen);
		//CursorControl = Win32CursorControl;

		// create driver

		createDriver(driverType, windowSize, bits, fullscreen, stencilbuffer, vsync, antiAlias, highPrecisionFPU);

		if (DeviceBase::VideoDriver_)
			DeviceBase::createGUIAndScene();

		// register environment
		environment_map_.insert(typename EnvironmentMap::value_type(HWnd,this));

		// set this as active window
		SetActiveWindow(HWnd);
		SetForegroundWindow(HWnd);
	}
	template<typename Configure>
	MADeviceWin32<Configure>::~MADeviceWin32()
	{
		typename EnvironmentMap::left_iterator envmap_it = environment_map_.left.find(HWnd);
		if (envmap_it != environment_map_.left.end())
		{
			environment_map_.left.erase(envmap_it);
		}
		if (ChangedToFullScreen)
		{
			ChangeDisplaySettings(NULL,0);
		}
	}
	template<typename Configure>
	bool MADeviceWin32<Configure>::execute()
	{
		//Timer::tick();
		MSG msg;
		bool quit = false;
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);

			if (ExternalWindow && msg.hwnd == HWnd)
				WndProc(HWnd, msg.message, msg.wParam, msg.lParam);
			else
				DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				quit = true;
		}

		if (!quit)
			resizeIfNecessary();

		return !quit;
	}
	template<typename Configure>
	void MADeviceWin32<Configure>::yield()
	{
		Sleep(10);
	}

	template<typename Configure>
	void MADeviceWin32<Configure>::sleep(std::size_t timeMs, bool is_pauseTimer)
	{
		bool wasStopped = DeviceBase::Timer_ ? DeviceBase::Timer_->isStopped() : true;
		if (is_pauseTimer && !wasStopped)
			DeviceBase::Timer_->stop();

		Sleep(timeMs);

		if (is_pauseTimer && !wasStopped)
			DeviceBase::Timer_->start();
	}

	template<typename Configure>
	void MADeviceWin32<Configure>::createDriver(DriverType driverType, const scalar2i& windowSize,
		unsigned int bits, bool fullscreen, bool stencilbuffer, bool vsync, bool antiAlias, bool highPrecisionFPU)
	{
		if (fullscreen)	switchToFullScreen(windowSize[0], windowSize[1], bits);
		typename Configure::DriverCreator driver_creator;
		DeviceBase::VideoDriver_ = driver_creator(windowSize, fullscreen, DeviceBase::FileSystem_);
	}
	template<typename Configure>
	void MADeviceWin32<Configure>::resizeIfNecessary()
	{

	}
	template<typename Configure>
	void MADeviceWin32<Configure>::OnResized()
	{

	}
	template<typename Configure>
	bool MADeviceWin32<Configure>::switchToFullScreen(int width, int height, int bits)
	{
		//...
		return false;
	}
	template<typename Configure>
	template<typename ImagePtr>
	void MADeviceWin32<Configure>::present(ImagePtr image, int windowId /* = 0 */, recti* src/* =0  */)
	{
		HWND hwnd = HWnd;
		if ( windowId )
			hwnd = reinterpret_cast<HWND>((void*)&windowId);//get rid of warning

		HDC dc = GetDC(hwnd);

		if ( dc )
		{
			RECT rect;
			GetClientRect(hwnd, &rect);
			const void* memory = (const void *)image->lock();

			BITMAPV4HEADER bi;
			ZeroMemory (&bi, sizeof(bi));
			bi.bV4Size          = sizeof(BITMAPINFOHEADER);
			bi.bV4BitCount      = image->getBitsPerPixel();
			bi.bV4Planes        = 1;
			bi.bV4Width         = scalar2_op::width(image->getDimension());
			bi.bV4Height        = -scalar2_op::height(image->getDimension());
			bi.bV4V4Compression = BI_BITFIELDS;
			bi.bV4AlphaMask     = image->getAlphaMask ();
			bi.bV4RedMask       = image->getRedMask ();
			bi.bV4GreenMask     = image->getGreenMask();
			bi.bV4BlueMask      = image->getBlueMask();

			if ( src )
			{
				using namespace rect_op;
				StretchDIBits(dc, 0,0, rect.right, rect.bottom,
					//src->UpperLeftCorner.X, src->UpperLeftCorner.Y,
					top(*src),left(*src),
					width(*src), height(*src),
					memory, (const BITMAPINFO*)(&bi), DIB_RGB_COLORS, SRCCOPY);
			}
			else
			{
				StretchDIBits(dc, 0,0, rect.right, rect.bottom,
					0, 0, scalar2_op::width(image->getDimension()), scalar2_op::height(image->getDimension()),
					memory, (const BITMAPINFO*)(&bi), DIB_RGB_COLORS, SRCCOPY);
			}

			image->unlock();

			ReleaseDC(hwnd, dc);
		}
	}
}

#endif

