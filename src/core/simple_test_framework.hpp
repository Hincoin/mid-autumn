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

#ifdef NDEBUG
//#define ENABLE_PERFORMANCE_TEST
#endif

#endif