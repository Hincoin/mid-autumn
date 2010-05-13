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

#ifndef INCLUDE_TIMER_HPP
#define INCLUDE_TIMER_HPP

#include <ctime>
#include <iostream>

#include "NullType.hpp"
namespace ma
{

	//! Interface for getting and manipulating the virtual time
	template<typename Derived,typename ClassConfig = NullType>
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
		unsigned int getRealTime() const {return derived().getRealTime();}

		//! Returns current virtual time in milliseconds.
		/** This value starts with 0 and can be manipulated using setTime(),
		stopTimer(), startTimer(), etc. This value depends on the set speed of
		the timer if the timer is stopped, etc. If you need the system time,
		use getRealTime() */
		 unsigned int getTime() const {return derived().getTime();}

		//! sets current virtual time
		 void setTime(unsigned int time) {return derived().setTime(time);}

		//! Stops the virtual timer.
		/** The timer is reference counted, which means everything which calls
		stop() will also have to call start(), otherwise the timer may not
		start/stop correctly again. */
		 void stop(){return derived().stop();}

		//! Starts the virtual timer.
		/** The timer is reference counted, which means everything which calls
		stop() will also have to call start(), otherwise the timer may not
		start/stop correctly again. */
		 void start() {return derived().start();}

		//! Sets the speed of the timer
		/** The speed is the factor with which the time is running faster or
		slower then the real system time. */
		 void setSpeed(float speed ) {return derived().setSpeed(speed);}

		//! Returns current speed of the timer
		/** The speed is the factor with which the time is running faster or
		slower then the real system time. */
		 float getSpeed() const {return derived().getSpeed();}

		//! Returns if the virtual timer is currently stopped
		 bool isStopped() const{return derived().isStopped();}

		//! Advances the virtual time
		/** Makes the virtual timer update the time value based on the real
		time. This is called automatically when calling IrrlichtDevice::run(),
		but you can call it manually if you don't use this method. */
		void tick() {return derived().tick();}
	};

	namespace perf{
		struct Timer{


			void start(){
				time_start_ = clock();}
			void end(){
				time_end_ = clock();
			}
			void stop(){
				printf("Timer ms:%.6f \n",1000.0 * ((double)time_end_-(double)time_start_)/double(CLOCKS_PER_SEC));
			}
		private:
			clock_t time_start_;
			clock_t time_end_;
		};

	}
}
#endif
