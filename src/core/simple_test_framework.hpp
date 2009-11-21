#ifndef SIMPLE_TEST_FRAME_WORK_HPP
#define SIMPLE_TEST_FRAME_WORK_HPP

#include <stdio.h>

typedef bool (*test_func_t)();

bool register_test_function(const char* module_name,test_func_t f);
bool run_test();

#include <cassert>

#define REGISTER_TEST_FUNC(NAME,FUNC)\
	namespace{\
	static bool _registered_test_function_ret_value_ = register_test_function(#NAME,FUNC); \
	\
	}

#include <time.h>


struct timer{
private:
	clock_t start_time;
public:
	void start_timer() { start_time = clock(); }

	double stop_timer() {
		return ( clock() - start_time)/ (double)(CLOCKS_PER_SEC);
	}  
	long clocks(){
		return  long(clock() - start_time) ;
	}
};


#ifdef NDEBUG
//#define ENABLE_PERFORMANCE_TEST
#endif

#endif