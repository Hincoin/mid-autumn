#ifndef MADEVICEWIN32IMPL_H
#define MADEVICEWIN32IMPL_H

#include "MADevice.h"

#include <winuser.h>


namespace ma{
	namespace details{
		
//		template<typename Configure>
//		LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//		{
//#ifndef WM_MOUSEWHEEL
//#define WM_MOUSEWHEEL 0x020A
//#endif
//#ifndef WHEEL_DELTA
//#define WHEEL_DELTA 120
//#endif
//			irr::CIrrDeviceWin32* dev = 0;
//			irr::SEvent event;
//			SEnvMapper* envm = 0;
//
//			BYTE allKeys[256];
//
//			static irr::s32 ClickCount=0;
//			if (GetCapture() != hWnd && ClickCount > 0)
//				ClickCount = 0;
//
//			switch (message)
//			{
//			case WM_PAINT:
//				{
//					PAINTSTRUCT ps;
//					BeginPaint(hWnd, &ps);
//					EndPaint(hWnd, &ps);
//				}
//				return 0;
//
//			case WM_ERASEBKGND:
//				return 0;
//
//			case WM_SETCURSOR:
//				envm = getEnvMapperFromHWnd(hWnd);
//				if (envm && !envm->irrDev->getWin32CursorControl()->isVisible())
//				{
//					SetCursor(NULL);
//					return 0;
//				}
//				break;
//
//			case WM_MOUSEWHEEL:
//				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
//				event.MouseInput.Wheel = ((irr::f32)((short)HIWORD(wParam))) / (irr::f32)WHEEL_DELTA;
//				event.MouseInput.Event = irr::EMIE_MOUSE_WHEEL;
//
//				POINT p; // fixed by jox
//				p.x = 0; p.y = 0;
//				ClientToScreen(hWnd, &p);
//				event.MouseInput.X = LOWORD(lParam) - p.x;
//				event.MouseInput.Y = HIWORD(lParam) - p.y;
//
//				dev = getDeviceFromHWnd(hWnd);
//				if (dev)
//					dev->postEventFromUser(event);
//				break;
//
//			case WM_LBUTTONDOWN:
//				ClickCount++;
//				SetCapture(hWnd);
//				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
//				event.MouseInput.Event = irr::EMIE_LMOUSE_PRESSED_DOWN;
//				event.MouseInput.X = (short)LOWORD(lParam);
//				event.MouseInput.Y = (short)HIWORD(lParam);
//				dev = getDeviceFromHWnd(hWnd);
//				if (dev)
//					dev->postEventFromUser(event);
//				return 0;
//
//			case WM_LBUTTONUP:
//				ClickCount--;
//				if (ClickCount<1)
//				{
//					ClickCount=0;
//					ReleaseCapture();
//				}
//				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
//				event.MouseInput.Event = irr::EMIE_LMOUSE_LEFT_UP;
//				event.MouseInput.X = (short)LOWORD(lParam);
//				event.MouseInput.Y = (short)HIWORD(lParam);
//				dev = getDeviceFromHWnd(hWnd);
//				if (dev)
//					dev->postEventFromUser(event);
//				return 0;
//
//			case WM_RBUTTONDOWN:
//				ClickCount++;
//				SetCapture(hWnd);
//				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
//				event.MouseInput.Event = irr::EMIE_RMOUSE_PRESSED_DOWN;
//				event.MouseInput.X = (short)LOWORD(lParam);
//				event.MouseInput.Y = (short)HIWORD(lParam);
//				dev = getDeviceFromHWnd(hWnd);
//				if (dev)
//					dev->postEventFromUser(event);
//				return 0;
//
//			case WM_RBUTTONUP:
//				ClickCount--;
//				if (ClickCount<1)
//				{
//					ClickCount=0;
//					ReleaseCapture();
//				}
//				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
//				event.MouseInput.Event = irr::EMIE_RMOUSE_LEFT_UP;
//				event.MouseInput.X = (short)LOWORD(lParam);
//				event.MouseInput.Y = (short)HIWORD(lParam);
//				dev = getDeviceFromHWnd(hWnd);
//				if (dev)
//					dev->postEventFromUser(event);
//				return 0;
//
//			case WM_MBUTTONDOWN:
//				ClickCount++;
//				SetCapture(hWnd);
//				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
//				event.MouseInput.Event = irr::EMIE_MMOUSE_PRESSED_DOWN;
//				event.MouseInput.X = (short)LOWORD(lParam);
//				event.MouseInput.Y = (short)HIWORD(lParam);
//				dev = getDeviceFromHWnd(hWnd);
//				if (dev)
//					dev->postEventFromUser(event);
//				return 0;
//
//			case WM_MBUTTONUP:
//				ClickCount--;
//				if (ClickCount<1)
//				{
//					ClickCount=0;
//					ReleaseCapture();
//				}
//				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
//				event.MouseInput.Event = irr::EMIE_MMOUSE_LEFT_UP;
//				event.MouseInput.X = (short)LOWORD(lParam);
//				event.MouseInput.Y = (short)HIWORD(lParam);
//				dev = getDeviceFromHWnd(hWnd);
//				if (dev)
//					dev->postEventFromUser(event);
//				return 0;
//
//			case WM_MOUSEMOVE:
//				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
//				event.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
//				event.MouseInput.X = (short)LOWORD(lParam);
//				event.MouseInput.Y = (short)HIWORD(lParam);
//				dev = getDeviceFromHWnd(hWnd);
//
//				if (dev)
//					dev->postEventFromUser(event);
//
//				return 0;
//
//			case WM_KEYDOWN:
//			case WM_KEYUP:
//				{
//					event.EventType = irr::EET_KEY_INPUT_EVENT;
//					event.KeyInput.Key = (irr::EKEY_CODE)wParam;
//					event.KeyInput.PressedDown = (message==WM_KEYDOWN);
//					dev = getDeviceFromHWnd(hWnd);
//
//					WORD KeyAsc=0;
//					GetKeyboardState(allKeys);
//					ToAscii((UINT)wParam,(UINT)lParam,allKeys,&KeyAsc,0);
//
//					event.KeyInput.Shift = ((allKeys[VK_SHIFT] & 0x80)!=0);
//					event.KeyInput.Control = ((allKeys[VK_CONTROL] & 0x80)!=0);
//					event.KeyInput.Char = (KeyAsc & 0x00ff); //KeyAsc >= 0 ? KeyAsc : 0;
//
//					if (dev)
//						dev->postEventFromUser(event);
//
//					return 0;
//				}
//
//			case WM_SIZE:
//				{
//					// resize
//					dev = getDeviceFromHWnd(hWnd);
//					if (dev)
//						dev->OnResized();
//				}
//				return 0;
//
//			case WM_DESTROY:
//				PostQuitMessage(0);
//				return 0;
//
//			case WM_SYSCOMMAND:
//				// prevent screensaver or monitor powersave mode from starting
//				if ((wParam & 0xFFF0) == SC_SCREENSAVE ||
//					(wParam & 0xFFF0) == SC_MONITORPOWER)
//					return 0;
//				break;
//			}
//
//			return DefWindowProc(hWnd, message, wParam, lParam);
//		}
	}
	template<typename Configure>
	bool MADeviceWin32<Configure>::execute()
	{
		return true;
	}
	template<typename Configure>
	void MADeviceWin32<Configure>::yield()
	{
	
	}
	
	template<typename Configure>
	void MADeviceWin32<Configure>::sleep(std::size_t timeMs, bool is_pauseTimer)
	{
	//
	}
}

#endif

