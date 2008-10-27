////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008 by luozhiyuan (luozhiyuan@gmail.com)
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author makes no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////


#ifndef INCLUDE_MADEVICE_BASE_HPP
#define INCLUDE_MADEVICE_BASE_HPP

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/noncopyable.hpp>
namespace ma{
 
	//render device
	template<typename Derived,typename Configure>
	class MADeviceBase: boost::noncopyable{
		//BOOST_STATIC_ASSERT((boost::is_base_and_derived<MADeviceBase<Derived,Configure>,Derived>::value));
	public:
		typedef typename Configure::VideoDriverPtr VideoDriverPtr;
		typedef typename Configure::FileSystemPtr FileSystemPtr;
		typedef typename Configure::GUIManagerPtr GUIManagerPtr;
		typedef typename Configure::SceneManagerPtr SceneManagerPtr;
		typedef typename Configure::LoggerPtr LoggerPtr;
		typedef typename Configure::VideoModePtr VideoModePtr;
		typedef typename Configure::OSOperatorPtr OSOperatorPtr;
		typedef typename Configure::TimerPtr TimerPtr;
		typedef typename Configure::EventProcessorPtr EventProcessorPtr;
		typedef typename Configure::EventType     EventType;
	protected:
		MADeviceBase(){}
		~MADeviceBase(){}
	private:
		inline const Derived& derived() const { return *static_cast<const Derived*>(this); }
		inline Derived& derived() { return *static_cast<Derived*>(this); }
	public:
		bool execute(){return static_cast<Derived&>(*this).execute();}

		//! Cause the device to temporarily pause execution and let other processes run.
		/** This should bring down processor usage without major
		performance loss */
		void yield(){static_cast<Derived&>(*this).yield();}

		//! Pause execution and let other processes to run for a specified amount of time.
		/** It may not wait the full given time, as sleep may be interrupted
		\param timeMs: Time to sleep for in milisecs.
		\param pauseTimer: If true, pauses the device timer while sleeping
		*/
		void sleep(std::size_t timeMs, bool pauseTimer){static_cast<Derived&>(*this).sleep(timeMs,pauseTimer);}

		//! Provides access to the video driver for drawing 3d and 2d geometry.
		/** \return Pointer the video driver. */
		
		VideoDriverPtr getVideoDriver(){return static_cast<Derived&>(*this).getVideoDriver();}

		//! Provides access to the virtual file system.
		/** \return Pointer to the file system. */
		FileSystemPtr getFileSystem() {return static_cast<Derived&>(*this).getFileSystem();}

		//! Provides access to the 2d user interface environment.
		/** \return Pointer to the gui environment. */
		GUIManagerPtr getGUIEnvironment() {return derived().getGUIEnvironment();}

		//! Provides access to the scene manager.
		/** \return Pointer to the scene manager. */
		SceneManagerPtr getSceneManager() {return derived().getSceneManager();}

		//! Provides access to the cursor control.
		/** \return Pointer to the mouse cursor control interface. */
		//virtual gui::ICursorControl* getCursorControl() ;

		//! Provides access to the message logger.
		/** \return Pointer to the logger. */
		LoggerPtr getLogger() {return derived().getLogger();}

		//! Gets a list with all video modes available.
		/** If you are confused now, because you think you have to
		create a Device with a video mode before being able
		to get the video mode list, let me tell you that there is no
		need to start up a Device with EDT_DIRECT3D8,
		EDT_OPENGL or EDT_SOFTWARE: For this (and for lots of other
		reasons) the null driver, EDT_NULL exists.
		\return Pointer to a list with all video modes supported
		by the gfx adapter. */
		VideoModePtr getVideoModes() {return derived().getVideoModes();}

		//! Provides access to the operation system operator object.
		/** The OS operator provides methods for
		getting system specific informations and doing system
		specific operations, such as exchanging data with the clipboard
		or reading the operation system version.
		\return Pointer to the OS operator. */
		OSOperatorPtr getOSOperator() {return derived().getOSOperator();}

		//! Provides access to the engine's timer.
		/** The system time can be retrieved by it as
		well as the virtual time, which also can be manipulated.
		\return Pointer to the ITimer object. */
		TimerPtr getTimer() {return derived().getTimer();}

		//! Sets the caption of the window.
		/** \param text: New text of the window caption. */
		//void setWindowCaption(const wchar_t* text) ;

		//! Returns if the window is active.
		/** If the window is inactive,
		nothing needs to be drawn. So if you don't want to draw anything
		when the window is inactive, create your drawing loop this way:
		\code
		while(device->run())
		{
		if (device->isWindowActive())
		{
		// draw everything here
		}
		else
		device->yield();
		}
		\endcode
		\return True if window is active. */
		bool isWindowActive() const {return derived().isWindowActive();}

		//! Notifies the device that it should close itself.
		/** MADevice::run() will always return false after closeDevice() was called. */
		void closeDevice() {return derived().closeDevice();}

		//! Get the version of the engine.
		/** The returned string
		will look like this: "1.2.3" or this: "1.2".
		\return String which contains the version. */
		static const char* getVersion(){return derived().getVersion();}

		//! Sets a new event receiver to receive events.
		/** \param receiver New receiver to be used. */
		void setEventReceiver(EventProcessorPtr receiver) 
		{
			derived().setEventReceiver(receiver);
		}

		//! Provides access to the current event receiver.
		/** \return Pointer to the current event receiver. Returns 0 if there is none. */
		EventProcessorPtr getEventReceiver() {return derived().getEventReceiver();}

		//! Sends a user created event to the engine.
		/** Is is usually not necessary to use this. However, if you
		are using an own input library for example for doing joystick
		input, you can use this to post key or mouse input events to
		the engine. Internally, this method only delegates the events
		further to the scene manager and the GUI environment. */
		void postEventFromUser(const EventType& event) 
		{
			return derived().postEventFromUser(event);
		};

		//! Sets the input receiving scene manager.
		/** If set to null, the main scene manager (returned by
		GetSceneManager()) will receive the input
		\param sceneManager New scene manager to be used. */
		void setInputReceivingSceneManager(SceneManagerPtr sceneManager) {return derived().setInputReceivingSceneManager(sceneManager);}

		//! Sets if the window should be resizeable in windowed mode.
		/** The default is false. This method only works in windowed
		mode.
		\param resize Flag whether the window should be resizeable. */
		void setResizeAble(bool resize) {derived().setResizeAble(resize);}

	};
}


#endif