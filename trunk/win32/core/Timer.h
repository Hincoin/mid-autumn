#ifndef TIMER_H
#define TIMER_H

#include <ctime>
#include <iostream>

namespace ma
{
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