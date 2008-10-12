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