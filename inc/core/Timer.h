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

#ifndef TIMER_H
#define TIMER_H

#include <ctime>
#include <iostream>

namespace ma
{

	//! Interface for getting and manipulating the virtual time
	template<typename Derived>
	class Timer
	{
		Derived& derived(){return static_cast<Derived&>(*this);}
		const Derived& derived()const{return static_cast<Derived&>(*this);}
	protected:
		Timer(){}
		~Timer(){}
	public:

		//! Returns current real time in milliseconds of the system.
		/** This value does not start with 0 when the application starts.
		For example in one implementation the value returned could be the
		amount of milliseconds which have elapsed since the system was started.
		*/
		virtual u32 getRealTime() const = 0;

		//! Returns current virtual time in milliseconds.
		/** This value starts with 0 and can be manipulated using setTime(),
		stopTimer(), startTimer(), etc. This value depends on the set speed of
		the timer if the timer is stopped, etc. If you need the system time,
		use getRealTime() */
		virtual u32 getTime() const = 0;

		//! sets current virtual time
		virtual void setTime(u32 time) = 0;

		//! Stops the virtual timer.
		/** The timer is reference counted, which means everything which calls
		stop() will also have to call start(), otherwise the timer may not
		start/stop correctly again. */
		virtual void stop() = 0;

		//! Starts the virtual timer.
		/** The timer is reference counted, which means everything which calls
		stop() will also have to call start(), otherwise the timer may not
		start/stop correctly again. */
		virtual void start() = 0;

		//! Sets the speed of the timer
		/** The speed is the factor with which the time is running faster or
		slower then the real system time. */
		virtual void setSpeed(f32 speed = 1.0f) = 0;

		//! Returns current speed of the timer
		/** The speed is the factor with which the time is running faster or
		slower then the real system time. */
		virtual f32 getSpeed() const = 0;

		//! Returns if the virtual timer is currently stopped
		virtual bool isStopped() const = 0;

		//! Advances the virtual time
		/** Makes the virtual timer update the time value based on the real
		time. This is called automatically when calling IrrlichtDevice::run(),
		but you can call it manually if you don't use this method. */
		virtual void tick() = 0;
	};

	namespace perf{
		struct Timer{

			
			void start(){
				time_start_ = clock();}
			void end(){
				time_end_ = clock();
			}
			void show(){
				printf("Timer clocks:%d \n",time_end_-time_start_);
			}
		private:
			clock_t time_start_;
			clock_t time_end_;
		};

	}
}
#endif