#ifndef MAX11DEVICE_HPP_INCLUDED
#define MAX11DEVICE_HPP_INCLUDED
#include "MAConfig.hpp"

#ifdef MA_X11_DEVICE
#include "MADevice.hpp"
#include "Vector.hpp"
#include <string>
#include <boost/bimap.hpp>
#include "SpaceSegment.hpp"


#include "MAEventType.hpp"
#include "Image.hpp"
#include "MAColor.hpp"
///////////////////////////////////////

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <X11/keysym.h>

/// implementation need
#include <sys/utsname.h>
#include <time.h>
#include <X11/XKBlib.h>
namespace ma
{
    template<typename Configure_T>
    class MADeviceX11:public MADevice<MADeviceX11<Configure_T>, Configure_T >,  Configure_T::ImagePresenter
    {
    public:
        typedef  MADevice<MADeviceX11<Configure_T>, Configure_T > BaseDevice;
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

        typedef MADevice<MADeviceX11<Configure_T>, Configure_T > DeviceBase;
    public:
        MADeviceX11(DriverType driverType,
                    scalar2i& windowSize,
                    unsigned int bits, bool fullscreen,
                    bool stencilbuffer, bool vsync,
                    bool antiAlias,
                    bool highPrecisionFPU,
                    EventProcessorPtr receiver,
                    int externalWindow);
        ~MADeviceX11();

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
        ///createWindow
        bool createWindow(const scalar2i& windowSize,unsigned bits);
        void createKeyMap();
        //! create the driver
        void createDriver(
            const scalar2i& windowSize,  bool vsync);

        //! switches to fullscreen
        bool switchToFullScreen(int width, int height, int bits);

        std::string getWindowsVersion(/*const std::string& version*/)
        {
            return "";
        }

        void resizeIfNecessary();

        Display* display;
        XVisualInfo* visual;
        int screennr;
        Window window;
        XSetWindowAttributes attributes;
        XSizeHints* StdHints;
        XImage* SoftwareImage;

        SizeID RandrMode;
        Rotation RandrRotation;



        bool ChangedToFullScreen;
        bool FullScreen;
        bool IsNonNTWindows;
        bool Resized;
        bool ExternalWindow;
        bool StencilBuffer;
        bool AntiAlias;
        unsigned Width,Height,Depth;
        bool Close;
        bool WindowActive;
        bool WindowMinimized;

        int AutorepeatSupport;
    private:
        typedef boost::bimap<KeySym,int> X11KeyMap;
        static  X11KeyMap KeyMap;
        static int X11KeytoMAKey(KeySym x11)
        {
            X11KeyMap::left_iterator it = KeyMap.left.find(x11);
            return it == KeyMap.left.end()? -1 : it->second;
        }
    };
    template<typename Configure>
    boost::bimap<KeySym,int> MADeviceX11<Configure>::KeyMap;
    const char* wmDeleteWindow = "WM_DELETE_WINDOW";
    template<typename Configure>
    MADeviceX11<Configure>::MADeviceX11(DriverType driverType,
                                        scalar2i& windowSize,
                                        unsigned int bits, bool fullscreen,
                                        bool stencilbuffer, bool vsync,
                                        bool antiAlias,
                                        bool highPrecisionFPU,
                                        EventProcessorPtr receiver,
                                        int externalWindow):BaseDevice(receiver),
            ///
            display(0),visual(0),screennr(0),window(0),StdHints(0),
            SoftwareImage(0),
            ///
            FullScreen(fullscreen),StencilBuffer(stencilbuffer),
            AntiAlias(antiAlias),//DriverType_(driverType),
            Width(scalar2_op::width(windowSize)),
            Height(scalar2_op::height(windowSize)),
            Depth(24),
            Close(false),WindowActive(false),WindowMinimized(false),
            AutorepeatSupport(0)
    {
        std::string linuxversion;
        struct utsname LinuxInfo;
        uname(&LinuxInfo);
        linuxversion += LinuxInfo.sysname;
        linuxversion += " ";
        linuxversion += LinuxInfo.release;
        linuxversion += " ";
        linuxversion += LinuxInfo.version;
        linuxversion += " ";
        linuxversion += LinuxInfo.machine;

        BaseDevice::OSOperator_ = new OSOperator(/*linuxversion*/);
        createKeyMap();

        if (!createWindow(windowSize,bits))
            return;
        createDriver(windowSize,vsync);
        if (!BaseDevice::VideoDriver_)
            return;
        //createGUIAndScene();
    }
    template<typename Configure>
    MADeviceX11<Configure>::~MADeviceX11()
    {
        if (StdHints)
            XFree(StdHints);
        if (display)
        {
            if (FullScreen)
            {
                XRRScreenConfiguration *config = XRRGetScreenInfo(display,DefaultRootWindow(display));
                XRRSetScreenConfig(display,config,DefaultRootWindow(display),RandrMode,
                                   RandrRotation,CurrentTime);
                XRRFreeScreenConfigInfo(config);
            }
            XDestroyImage(SoftwareImage);
            XDestroyWindow(display,window);
            XCloseDisplay(display);

        }
        if (visual)
            XFree(visual);

    }
    template<typename Configure>
    bool MADeviceX11<Configure>::createWindow(const scalar2i&
            windowSize,
            unsigned bits)
    {
        Width = scalar2_op::width(windowSize);
        Height = scalar2_op::height(windowSize);
        display = XOpenDisplay(0);
        if (!display)
        {
            return false;
        }
        screennr = DefaultScreen(display);
        if (FullScreen)
        {
            int eventbase,errorbase;
            int bestMode = -1;
            int defaultDepth = DefaultDepth(display,screennr);
            if (XRRQueryExtension(display,&eventbase,&errorbase))
            {
                int modeCount;
                XRRScreenConfiguration *config=XRRGetScreenInfo(display,DefaultRootWindow(display));
                RandrMode = XRRConfigCurrentConfiguration(config,&RandrRotation);
                XRRScreenSize *modes = XRRConfigSizes(config,&modeCount);
                BaseDevice::VideoModeList_->setDesktop(defaultDepth,scalar2i(modes[RandrMode].width,modes[RandrMode].height));
                for (int i = 0;i<modeCount;++i)
                {
                    if (bestMode == -1 &&
                            modes[i].width >= (int)Width &&  modes[i].height >= (int)Height)
                        bestMode = i;
                    else if (bestMode!= -1 && modes[i].width >= (int)Width &&
                             modes[i].height >= (int)Height && modes[i].height < modes[bestMode].height
                             && modes[i].width < modes[bestMode].width)
                        bestMode = i;
                    BaseDevice::VideoModeList_->addMode(scalar2i(modes[i].width,modes[i].height),defaultDepth);
                }
                if (bestMode != -1)
                {
                    XRRSetScreenConfig(display,config,DefaultRootWindow(display),bestMode,RandrMode,CurrentTime);
                }
                XRRFreeScreenConfigInfo(config);
            }
            else
            {
                FullScreen = false;
            }
        }
        if (!visual)
        {
            XVisualInfo visTempl;
            int visNumber;
            visTempl.screen = screennr;
            visTempl.depth = 16;
            while ((!visual) && (visTempl.depth <= 32))
            {
                visual = XGetVisualInfo(display,VisualScreenMask|VisualDepthMask,
                                        &visTempl,&visNumber);
                visTempl.depth+=8;
            }
        }
        if (!visual)
        {
            XCloseDisplay(display);
            display = 0;
            return false;
        }

        ///create colormap
        Colormap colormap;
        colormap = XCreateColormap(display,
                                   RootWindow(display,visual->screen),visual->visual,
                                   AllocNone);
        attributes.colormap = colormap;
        attributes.border_pixel = 0;
        attributes.event_mask = KeyPressMask|ButtonPressMask
                                |StructureNotifyMask|PointerMotionMask|ButtonReleaseMask|
                                KeyReleaseMask;
        ///create window
        if (FullScreen)
        {
            attributes.override_redirect = True;
            window = XCreateWindow(display,RootWindow(display,visual->screen),
                                   0,0,Width,Height,0,visual->depth,
                                   InputOutput,visual->visual,
                                   CWBorderPixel | CWColormap | CWEventMask |
                                   CWOverrideRedirect,&attributes);
            XWarpPointer(display, None,window,0,0,0,0,0,0);
            XMapRaised(display,window);
            XGrabKeyboard(display,window,True,GrabModeAsync,
                          GrabModeAsync,CurrentTime);
            XGrabPointer(display,window,True,ButtonPressMask,
                         GrabModeAsync,GrabModeAsync,window,None,CurrentTime);
        }
        else
        {
            attributes.event_mask |= ExposureMask;
            attributes.event_mask |= FocusChangeMask;
            window = XCreateWindow(display,
                                   RootWindow(display,visual->screen),
                                   0,0,Width,Height,0,visual->depth,InputOutput,visual->visual,
                                   CWBorderPixel | CWColormap | CWEventMask,&attributes);
            Atom wmDelete ;

            wmDelete = XInternAtom(display,wmDeleteWindow,True);
            XSetWMProtocols(display,window,&wmDelete,1);
            XMapRaised(display,window);
        }
        WindowActive = true;
        XkbSetDetectableAutoRepeat(display,True,&AutorepeatSupport);
        // XkbSetDetectableAutoRepeat
        Window tmp;
        unsigned int borderWidth;
        int x,y;
        XGetGeometry(display,window,&tmp,&x,&y,&Width,&Height,&borderWidth,
                     &Depth);
        StdHints = XAllocSizeHints();
        long num;
        XGetWMNormalHints(display,window,StdHints,&num);
        ///create an XImage for the software renderer

        SoftwareImage = XCreateImage(display,
                                     visual->visual,visual->depth,
                                     ZPixmap,0,0,Width,Height,
                                     BitmapPad(display),0);
        SoftwareImage->data =(char*)malloc(SoftwareImage->bytes_per_line * SoftwareImage->height * sizeof(char));

        return true;
    }

    template<typename Configure>
    void MADeviceX11<Configure>::createDriver(
        const scalar2i& windowSize,  bool vsync)
    {
        //if (FullScreen)	switchToFullScreen(windowSize[0], windowSize[1], bits);
        typename Configure::DriverCreator driver_creator;
        BaseDevice::VideoDriver_ = driver_creator(windowSize, FullScreen, BaseDevice::FileSystem_);
    }
    /// main loop
    template<typename Configure>
    bool MADeviceX11<Configure>::execute()
    {
        if (display)
        {
            typedef typename Configure::EventType EventType;
            EventType cur_event;
            while (XPending(display) > 0 && ! Close)
            {
                XEvent event;
                XNextEvent(display, &event);

                switch (event.type)
                {
                case ConfigureNotify:
                    // check for changed window size
                    if ((event.xconfigure.width != (int) Width) ||
                            (event.xconfigure.height != (int) Height))
                    {
                        Width = event.xconfigure.width;
                        Height = event.xconfigure.height;

                        // resize image data
                        ///if (DriverType == video::EDT_SOFTWARE || DriverType == video::EDT_BURNINGSVIDEO)
                        {
                            XDestroyImage(SoftwareImage);

                            SoftwareImage = XCreateImage(display,
                                                         visual->visual, visual->depth,
                                                         ZPixmap, 0, 0, Width, Height,
                                                         BitmapPad(display), 0);

                            // use malloc because X will free it later on
                            SoftwareImage->data = (char*) malloc(SoftwareImage->bytes_per_line * SoftwareImage->height * sizeof(char));
                        }

                        if (BaseDevice::VideoDriver_)
                            BaseDevice::VideoDriver_->OnResize(scalar2i((int)Width,(int) Height));
                    }
                    break;

                case MapNotify:
                    WindowMinimized=false;
                    break;

                case UnmapNotify:
                    WindowMinimized=true;
                    break;

                case FocusIn:
                    WindowActive=true;
                    break;

                case FocusOut:
                    WindowActive=false;
                    break;

                case MotionNotify:
                    cur_event.event_type = EET_MOUSE_INPUT_EVENT;
                    cur_event.mouse_event.Event = EMIE_MOUSE_MOVED;
                    cur_event.mouse_event.x = event.xbutton.x;
                    cur_event.mouse_event.y = event.xbutton.y;

                    postEventFromUser(cur_event);
                    break;

                case ButtonPress:
                case ButtonRelease:

                    cur_event.event_type = EET_MOUSE_INPUT_EVENT;
                    cur_event.mouse_event.x = event.xbutton.x;
                    cur_event.mouse_event.y = event.xbutton.y;

                    cur_event.mouse_event.Event = EMIE_COUNT;

                    switch (event.xbutton.button)
                    {
                    case  Button1:
                        cur_event.mouse_event.Event =
                            (event.type == ButtonPress) ? EMIE_LMOUSE_DOWN : EMIE_LMOUSE_UP;
                        break;

                    case  Button3:
                        cur_event.mouse_event.Event =
                            (event.type == ButtonPress) ? EMIE_RMOUSE_DOWN : EMIE_RMOUSE_UP;
                        break;

                    case  Button2:
                        cur_event.mouse_event.Event =
                            (event.type == ButtonPress) ? EMIE_MMOUSE_DOWN : EMIE_MMOUSE_UP;
                        break;

                    case  Button4:
                        cur_event.mouse_event.Event = EMIE_MOUSE_WHEEL;
                        cur_event.mouse_event.Wheel = 1.0f;
                        break;

                    case  Button5:
                        cur_event.mouse_event.Event = EMIE_MOUSE_WHEEL;
                        cur_event.mouse_event.Wheel = -1.0f;
                        break;
                    }

                    if (cur_event.mouse_event.Event != EMIE_COUNT)
                        postEventFromUser(cur_event);
                    break;

                case MappingNotify:
                    XRefreshKeyboardMapping (&event.xmapping) ;
                    break;

                case KeyRelease:
                    if (0 == AutorepeatSupport)
                    {
                        // check for Autorepeat manually
                        // We'll do the same as Windows does: Only send KeyPressed
                        // So every KeyRelease is a real release
                        XEvent next_event;
                        XPeekEvent (event.xkey.display, &next_event);
                        if ((next_event.type == KeyPress) &&
                                (next_event.xkey.keycode == event.xkey.keycode) &&
                                (next_event.xkey.time == event.xkey.time))
                        {
                            /* Ignore the key release event */
                            break;
                        }
                    }
                    // fall-through in case the release should be handled
                case KeyPress:
                {
                    KeySym x11key;
                    //mp.X11Key = XLookupKeysym(&event.xkey, 0);
                    char buf[5]="\0\0\0\0";
                    XLookupString (&event.xkey, buf, 4, &x11key, NULL) ;

                    int key = X11KeytoMAKey(x11key);

                    if (key != -1)
                        cur_event.key_event.Key = (EKEY_CODE)key;
                    else
                    {
                        cur_event.key_event.Key = (EKEY_CODE)0;
                        //os::Printer::log("Could not find win32 key for x11 key.", ELL_WARNING);
                    }
                    cur_event.event_type = EET_KEY_INPUT_EVENT;
                    cur_event.key_event.PressedDown = (event.type == KeyPress);
                    mbtowc(&cur_event.key_event.Char, buf, 4);
                    cur_event.key_event.Control = (event.xkey.state & ControlMask) != 0;
                    cur_event.key_event.Shift = (event.xkey.state & ShiftMask) != 0;
                    postEventFromUser(cur_event);
                }
                break;

                case ClientMessage:
                {
                    char *atom = XGetAtomName(display, event.xclient.message_type);
                    if (*atom == *wmDeleteWindow)
                    {
                        ///os::Printer::log("Quit message received.", ELL_INFORMATION);
                        Close = true;
                    }
                    XFree(atom);
                }
                break;

                default:
                    break;
                } // end switch

            } // end while
        }
        return !Close;
    }




//! Pause the current process for the minimum time allowed only to allow other processes to execute
    template<typename Configure>
    void MADeviceX11<Configure>::yield()
    {
        struct timespec ts =
        {
            0,0
        };
        nanosleep(&ts, NULL);
    }

//! Pause execution and let other processes to run for a specified amount of time.
    template<typename Configure>
    void MADeviceX11<Configure>::sleep(unsigned timeMs, bool pauseTimer)
    {
        bool wasStopped = BaseDevice::Timer_ ? BaseDevice::Timer_->isStopped() : true;

        struct timespec ts;
        ts.tv_sec = (time_t) (timeMs / 1000);
        ts.tv_nsec = (long) (timeMs % 1000) * 1000000;

        if (pauseTimer && !wasStopped)
            BaseDevice::Timer_->stop();

        nanosleep(&ts, NULL);

        if (pauseTimer && !wasStopped)
            BaseDevice::Timer_->start();
    }

//! sets the caption of the window
    template<typename Configure>
    void MADeviceX11<Configure>::setWindowCaption(const wchar_t* text)
    {
        XTextProperty txt;
        XwcTextListToTextProperty(display, const_cast<wchar_t**>(&text), 1, XStdICCTextStyle, &txt);
        XSetWMName(display, window, &txt);
        XSetWMIconName(display, window, &txt);
    }



//! presents a surface in the client area
    template<typename Configure>
    template<typename ImagePtr>
    void MADeviceX11<Configure>::present(ImagePtr image, int windowId, recti* src )
    {
        // this is only necessary for software drivers.

        // thx to Nadav, who send me some clues of how to display the image
        // to the X Server.

        if (image->getColorFormat() != ma::ECF_A1R5G5B5 &&
                image->getColorFormat() != ma::ECF_A8R8G8B8)
        {
            ///os::Printer::log("Internal error, can only present A1R5G5B5 and A8R8G8B8 pictures.");
            return;
        }

        int destwidth = SoftwareImage->width;
        int destheight = SoftwareImage->height;
        int srcwidth = scalar2_op::width(image->getDimension());
        int srcheight = scalar2_op::height(image->getDimension());
        // clip images
        srcheight = srcheight < destheight ? srcheight : destheight;

        if ( image->getColorFormat() == ma::ECF_A8R8G8B8 )
        {
            // display 24/32 bit image

            int* srcdata = (int*)image->lock();

            if ((Depth == 32)||(Depth == 24))
            {
                int destPitch = SoftwareImage->bytes_per_line;
                unsigned char* destData = reinterpret_cast<unsigned char*>(SoftwareImage->data);

                for (int y=0; y<srcheight; ++y)
                {

                    color_op::convert_A8R8G8B8toA8R8G8B8(srcdata,srcwidth<destwidth?srcwidth:destwidth,destData);
                    srcdata+=srcwidth;
                    destData+=destPitch;
                }
            }
            else
                if (Depth == 16)
                {
                    // convert to R5G6B6

                    int destPitch = SoftwareImage->bytes_per_line;
                    unsigned char* destData = reinterpret_cast<unsigned char*>(SoftwareImage->data);

                    for (int y=0; y<srcheight; ++y)
                    {
                        color_op::convert_A8R8G8B8toR5G6B5(srcdata,srcwidth<destwidth?srcwidth:destwidth,destData);
                        srcdata+=srcwidth;
                        destData+=destPitch;
                    }
                }
                else
                    //os::Printer::log("Unsupported screen depth.");
                    printf("Unsupported screen depth.");

            image->unlock();
        }
        else
        {
            // display 16 bit image

            short* srcdata = (short*)image->lock();

            if (Depth == 16)
            {
                // convert from A1R5G5B5 to R5G6B6

                int destPitch = SoftwareImage->bytes_per_line;
                unsigned char* destData = reinterpret_cast<unsigned char*>(SoftwareImage->data);

                for (int y=0; y<srcheight; ++y)
                {
                    color_op::convert_A1R5G5B5toR5G6B5(srcdata,srcwidth<destwidth?srcwidth:destwidth,destData);
                    srcdata+=srcwidth;
                    destData+=destPitch;
                }
            }
            else
                if ((Depth == 32)||(Depth == 24))
                {
                    // convert from A1R5G5B5 to X8R8G8B8

                    int destPitch = SoftwareImage->bytes_per_line;
                    unsigned char* destData = reinterpret_cast<unsigned char*>(SoftwareImage->data);


                    for (int y=0; y<srcheight; ++y)
                    {
                        color_op::convert_A1R5G5B5toA8R8G8B8(srcdata,srcwidth<destwidth?srcwidth:destwidth,destData);
                        srcdata+=srcwidth;
                        destData+=destPitch;
                    }
                }
                else
                    printf("Unsupported screen depth.");

            image->unlock();
        }
        GC gc = DefaultGC(display, DefaultScreen(display));
        XPutImage(display, window, gc, SoftwareImage, 0, 0, 0, 0, destwidth, destheight);
    }



//! notifies the device that it should close itself
    template<typename Configure>
    void MADeviceX11<Configure>::closeDevice()
    {
        Close = true;
    }



//! returns if window is active. if not, nothing need to be drawn
    template<typename Configure>
    bool MADeviceX11<Configure>::isWindowActive() const
    {
        return WindowActive;
    }



//! Sets if the window should be resizeable in windowed mode.
    template<typename Configure>
    void MADeviceX11<Configure>::setResizeAble(bool resize)
    {
        XUnmapWindow(display, window);
        if ( !resize )
        {
            // Must be heap memory because data size depends on X Server
            XSizeHints *hints = XAllocSizeHints();
            hints->flags=PSize|PMinSize|PMaxSize;
            hints->min_width=hints->max_width=hints->base_width=Width;
            hints->min_height=hints->max_height=hints->base_height=Height;
            XSetWMNormalHints(display, window, hints);
            XFree(hints);
        }
        else
        {
            XSetWMNormalHints(display, window, StdHints);
        }
        XMapWindow(display, window);
        XFlush(display);
    }


//! \return Returns a pointer to a list with all video modes supported
////! by the gfx adapter.
//video::IVideoModeList* CIrrDeviceLinux::getVideoModeList()
//{
//#ifdef _IRR_COMPILE_WITH_X11_
//	if (!VideoModeList.getVideoModeCount())
//	{
//		bool temporaryDisplay = false;
//
//		if (!display)
//		{
//			display = XOpenDisplay(0);
//			temporaryDisplay=true;
//		}
//		if (display)
//		{
//			int eventbase, errorbase;
//			int defaultDepth=DefaultDepth(display,screennr);
//
//			#ifdef _IRR_LINUX_X11_VIDMODE_
//			if (XF86VidModeQueryExtension(display, &eventbase, &errorbase))
//			{
//				// enumerate video modes
//				int modeCount;
//				XF86VidModeModeInfo** modes;
//
//				XF86VidModeGetAllModeLines(display, screennr, &modeCount, &modes);
//
//				// save current video mode
//				oldVideoMode = *modes[0];
//
//				// find fitting mode
//
//				VideoModeList.setDesktop(defaultDepth, core::dimension2d<int>(
//					modes[0]->hdisplay, modes[0]->vdisplay));
//				for (int i = 0; i<modeCount; ++i)
//				{
//					VideoModeList.addMode(core::dimension2d<int>(
//						modes[i]->hdisplay, modes[i]->vdisplay), defaultDepth);
//				}
//				XFree(modes);
//			}
//			else
//			#endif
//			#ifdef _IRR_LINUX_X11_RANDR_
//			if (XRRQueryExtension(display, &eventbase, &errorbase))
//			{
//				int modeCount;
//				XRRScreenConfiguration *config=XRRGetScreenInfo(display,DefaultRootWindow(display));
//				oldRandrMode=XRRConfigCurrentConfiguration(config,&oldRandrRotation);
//				XRRScreenSize *modes=XRRConfigSizes(config,&modeCount);
//				VideoModeList.setDesktop(defaultDepth, core::dimension2d<int>(
//					modes[oldRandrMode].width, modes[oldRandrMode].height));
//				for (int i = 0; i<modeCount; ++i)
//				{
//					VideoModeList.addMode(core::dimension2d<int>(
//						modes[i].width, modes[i].height), defaultDepth);
//				}
//				XRRFreeScreenConfigInfo(config);
//			}
//			else
//			#endif
//			{
//				os::Printer::log("VidMode or RandR X11 extension requireed for VideoModeList." , ELL_WARNING);
//			}
//		}
//		if (display && temporaryDisplay)
//		{
//			XCloseDisplay(display);
//		}
//	}
//#endif
//
//	return &VideoModeList;
//}


    template<typename Configure>
    void MADeviceX11<Configure>::createKeyMap()
    {
        // I don't know if this is the best method  to create
        // the lookuptable, but I'll leave it like that until
        // I find a better version.

        KeyMap.insert(typename X11KeyMap::value_type(XK_BackSpace,KEY_BACK));
        KeyMap.insert(typename X11KeyMap::value_type(XK_BackSpace, KEY_BACK));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Tab, KEY_TAB));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Linefeed, 0)); // ???
        KeyMap.insert(typename X11KeyMap::value_type(XK_Clear, KEY_CLEAR));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Return, KEY_RETURN));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Pause, KEY_PAUSE));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Scroll_Lock, KEY_SCROLL));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Sys_Req, 0)); // ???
        KeyMap.insert(typename X11KeyMap::value_type(XK_Escape, KEY_ESCAPE));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Delete, KEY_DELETE));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Home, KEY_HOME));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Left, KEY_LEFT));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Up, KEY_UP));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Right, KEY_RIGHT));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Down, KEY_DOWN));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Prior, KEY_PRIOR));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Page_Up, KEY_PRIOR));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Next, KEY_NEXT));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Page_Down, KEY_NEXT));
        KeyMap.insert(typename X11KeyMap::value_type(XK_End, KEY_END));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Begin, KEY_HOME));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Space, KEY_SPACE));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Tab, KEY_TAB));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Enter, KEY_RETURN));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_F1, KEY_F1));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_F2, KEY_F2));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_F3, KEY_F3));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_F4, KEY_F4));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Left, KEY_LEFT));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Up, KEY_UP));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Right, KEY_RIGHT));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Down, KEY_DOWN));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Prior, KEY_PRIOR));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Page_Up, KEY_PRIOR));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Next, KEY_NEXT));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Page_Down, KEY_NEXT));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_End, KEY_END));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Begin, KEY_HOME));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Insert, KEY_INSERT));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Delete, KEY_DELETE));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Equal, 0)); // ???
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Multiply, KEY_MULTIPLY));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Add, KEY_ADD));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Separator, KEY_SEPARATOR));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Subtract, KEY_SUBTRACT));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Decimal, KEY_DECIMAL));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_Divide, KEY_DIVIDE));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_0, KEY_KEY_0));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_1, KEY_KEY_1));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_2, KEY_KEY_2));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_3, KEY_KEY_3));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_4, KEY_KEY_4));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_5, KEY_KEY_5));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_6, KEY_KEY_6));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_7, KEY_KEY_7));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_8, KEY_KEY_8));
        KeyMap.insert(typename X11KeyMap::value_type(XK_KP_9, KEY_KEY_9));
        KeyMap.insert(typename X11KeyMap::value_type(XK_F1, KEY_F1));
        KeyMap.insert(typename X11KeyMap::value_type(XK_F2, KEY_F2));
        KeyMap.insert(typename X11KeyMap::value_type(XK_F3, KEY_F3));
        KeyMap.insert(typename X11KeyMap::value_type(XK_F4, KEY_F4));
        KeyMap.insert(typename X11KeyMap::value_type(XK_F5, KEY_F5));
        KeyMap.insert(typename X11KeyMap::value_type(XK_F6, KEY_F6));
        KeyMap.insert(typename X11KeyMap::value_type(XK_F7, KEY_F7));
        KeyMap.insert(typename X11KeyMap::value_type(XK_F8, KEY_F8));
        KeyMap.insert(typename X11KeyMap::value_type(XK_F9, KEY_F9));
        KeyMap.insert(typename X11KeyMap::value_type(XK_F10, KEY_F10));
        KeyMap.insert(typename X11KeyMap::value_type(XK_F11, KEY_F11));
        KeyMap.insert(typename X11KeyMap::value_type(XK_F12, KEY_F12));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Shift_L, KEY_LSHIFT));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Shift_R, KEY_RSHIFT));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Control_L, KEY_LCONTROL));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Control_R, KEY_RCONTROL));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Caps_Lock, KEY_CAPITAL));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Shift_Lock, KEY_CAPITAL));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Meta_L, KEY_LWIN));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Meta_R, KEY_RWIN));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Alt_L, KEY_LMENU));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Alt_R, KEY_RMENU));
        KeyMap.insert(typename X11KeyMap::value_type(XK_ISO_Level3_Shift, KEY_RMENU));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Menu, KEY_MENU));
        KeyMap.insert(typename X11KeyMap::value_type(XK_space, KEY_SPACE));
        KeyMap.insert(typename X11KeyMap::value_type(XK_exclam, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_quotedbl, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_section, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_numbersign, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_dollar, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_percent, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_ampersand, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_apostrophe, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_parenleft, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_parenright, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_asterisk, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_plus, KEY_PLUS)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_comma, KEY_COMMA)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_minus, KEY_MINUS)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_period, KEY_PERIOD)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_slash, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_0, KEY_KEY_0));
        KeyMap.insert(typename X11KeyMap::value_type(XK_1, KEY_KEY_1));
        KeyMap.insert(typename X11KeyMap::value_type(XK_2, KEY_KEY_2));
        KeyMap.insert(typename X11KeyMap::value_type(XK_3, KEY_KEY_3));
        KeyMap.insert(typename X11KeyMap::value_type(XK_4, KEY_KEY_4));
        KeyMap.insert(typename X11KeyMap::value_type(XK_5, KEY_KEY_5));
        KeyMap.insert(typename X11KeyMap::value_type(XK_6, KEY_KEY_6));
        KeyMap.insert(typename X11KeyMap::value_type(XK_7, KEY_KEY_7));
        KeyMap.insert(typename X11KeyMap::value_type(XK_8, KEY_KEY_8));
        KeyMap.insert(typename X11KeyMap::value_type(XK_9, KEY_KEY_9));
        KeyMap.insert(typename X11KeyMap::value_type(XK_colon, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_semicolon, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_less, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_equal, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_greater, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_question, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_at, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_mu, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_EuroSign, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_A, KEY_KEY_A));
        KeyMap.insert(typename X11KeyMap::value_type(XK_B, KEY_KEY_B));
        KeyMap.insert(typename X11KeyMap::value_type(XK_C, KEY_KEY_C));
        KeyMap.insert(typename X11KeyMap::value_type(XK_D, KEY_KEY_D));
        KeyMap.insert(typename X11KeyMap::value_type(XK_E, KEY_KEY_E));
        KeyMap.insert(typename X11KeyMap::value_type(XK_F, KEY_KEY_F));
        KeyMap.insert(typename X11KeyMap::value_type(XK_G, KEY_KEY_G));
        KeyMap.insert(typename X11KeyMap::value_type(XK_H, KEY_KEY_H));
        KeyMap.insert(typename X11KeyMap::value_type(XK_I, KEY_KEY_I));
        KeyMap.insert(typename X11KeyMap::value_type(XK_J, KEY_KEY_J));
        KeyMap.insert(typename X11KeyMap::value_type(XK_K, KEY_KEY_K));
        KeyMap.insert(typename X11KeyMap::value_type(XK_L, KEY_KEY_L));
        KeyMap.insert(typename X11KeyMap::value_type(XK_M, KEY_KEY_M));
        KeyMap.insert(typename X11KeyMap::value_type(XK_N, KEY_KEY_N));
        KeyMap.insert(typename X11KeyMap::value_type(XK_O, KEY_KEY_O));
        KeyMap.insert(typename X11KeyMap::value_type(XK_P, KEY_KEY_P));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Q, KEY_KEY_Q));
        KeyMap.insert(typename X11KeyMap::value_type(XK_R, KEY_KEY_R));
        KeyMap.insert(typename X11KeyMap::value_type(XK_S, KEY_KEY_S));
        KeyMap.insert(typename X11KeyMap::value_type(XK_T, KEY_KEY_T));
        KeyMap.insert(typename X11KeyMap::value_type(XK_U, KEY_KEY_U));
        KeyMap.insert(typename X11KeyMap::value_type(XK_V, KEY_KEY_V));
        KeyMap.insert(typename X11KeyMap::value_type(XK_W, KEY_KEY_W));
        KeyMap.insert(typename X11KeyMap::value_type(XK_X, KEY_KEY_X));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Y, KEY_KEY_Y));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Z, KEY_KEY_Z));
        KeyMap.insert(typename X11KeyMap::value_type(XK_Adiaeresis, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_Odiaeresis, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_Udiaeresis, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_bracketleft, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_backslash, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_bracketright, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_asciicircum, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_degree, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_underscore, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_grave, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_acute, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_quoteleft, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_a, KEY_KEY_A));
        KeyMap.insert(typename X11KeyMap::value_type(XK_b, KEY_KEY_B));
        KeyMap.insert(typename X11KeyMap::value_type(XK_c, KEY_KEY_C));
        KeyMap.insert(typename X11KeyMap::value_type(XK_d, KEY_KEY_D));
        KeyMap.insert(typename X11KeyMap::value_type(XK_e, KEY_KEY_E));
        KeyMap.insert(typename X11KeyMap::value_type(XK_f, KEY_KEY_F));
        KeyMap.insert(typename X11KeyMap::value_type(XK_g, KEY_KEY_G));
        KeyMap.insert(typename X11KeyMap::value_type(XK_h, KEY_KEY_H));
        KeyMap.insert(typename X11KeyMap::value_type(XK_i, KEY_KEY_I));
        KeyMap.insert(typename X11KeyMap::value_type(XK_j, KEY_KEY_J));
        KeyMap.insert(typename X11KeyMap::value_type(XK_k, KEY_KEY_K));
        KeyMap.insert(typename X11KeyMap::value_type(XK_l, KEY_KEY_L));
        KeyMap.insert(typename X11KeyMap::value_type(XK_m, KEY_KEY_M));
        KeyMap.insert(typename X11KeyMap::value_type(XK_n, KEY_KEY_N));
        KeyMap.insert(typename X11KeyMap::value_type(XK_o, KEY_KEY_O));
        KeyMap.insert(typename X11KeyMap::value_type(XK_p, KEY_KEY_P));
        KeyMap.insert(typename X11KeyMap::value_type(XK_q, KEY_KEY_Q));
        KeyMap.insert(typename X11KeyMap::value_type(XK_r, KEY_KEY_R));
        KeyMap.insert(typename X11KeyMap::value_type(XK_s, KEY_KEY_S));
        KeyMap.insert(typename X11KeyMap::value_type(XK_t, KEY_KEY_T));
        KeyMap.insert(typename X11KeyMap::value_type(XK_u, KEY_KEY_U));
        KeyMap.insert(typename X11KeyMap::value_type(XK_v, KEY_KEY_V));
        KeyMap.insert(typename X11KeyMap::value_type(XK_w, KEY_KEY_W));
        KeyMap.insert(typename X11KeyMap::value_type(XK_x, KEY_KEY_X));
        KeyMap.insert(typename X11KeyMap::value_type(XK_y, KEY_KEY_Y));
        KeyMap.insert(typename X11KeyMap::value_type(XK_z, KEY_KEY_Z));
        KeyMap.insert(typename X11KeyMap::value_type(XK_ssharp, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_adiaeresis, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_odiaeresis, 0)); //?
        KeyMap.insert(typename X11KeyMap::value_type(XK_udiaeresis, 0)); //?
    }


}


#endif
#endif // MAX20DEVICE_HPP_INCLUDED
