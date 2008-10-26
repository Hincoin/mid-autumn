#ifndef MA_TIMER_WIN32IMPL_HPP
#define MA_TIMER_WIN32IMPL_HPP

#include "Timer.hpp"
namespace ma{
	class MATimerWin32:public Timer<MATimerWin32>
	{
	public:

		//! Returns current real time in milliseconds of the system.
		/** This value does not start with 0 when the application starts.
		For example in one implementation the value returned could be the
		amount of milliseconds which have elapsed since the system was started.
		*/
		unsigned int getRealTime() const {return 0;}

		//! Returns current virtual time in milliseconds.
		/** This value starts with 0 and can be manipulated using setTime(),
		stopTimer(), startTimer(), etc. This value depends on the set speed of
		the timer if the timer is stopped, etc. If you need the system time,
		use getRealTime() */
		unsigned int getTime() const {return 0;}

		//! sets current virtual time
		void setTime(unsigned int time) {return 0;}

		//! Stops the virtual timer.
		/** The timer is reference counted, which means everything which calls
		stop() will also have to call start(), otherwise the timer may not
		start/stop correctly again. */
		void stop(){return  ;}

		//! Starts the virtual timer.
		/** The timer is reference counted, which means everything which calls
		stop() will also have to call start(), otherwise the timer may not
		start/stop correctly again. */
		void start() {return  ;}

		//! Sets the speed of the timer
		/** The speed is the factor with which the time is running faster or
		slower then the real system time. */
		void setSpeed(float speed ) {return ;}

		//! Returns current speed of the timer
		/** The speed is the factor with which the time is running faster or
		slower then the real system time. */
		float getSpeed() const {return 0;}

		//! Returns if the virtual timer is currently stopped
		bool isStopped() const{return false;}

		//! Advances the virtual time
		/** Makes the virtual timer update the time value based on the real
		time. This is called automatically when calling IrrlichtDevice::run(),
		but you can call it manually if you don't use this method. */
		void tick() {return ;}
	};
}

#endif