#include "simple_test_framework.hpp"

bool virtual_interface_test();


REGISTER_TEST_FUNC(virtual_interface,virtual_interface_test)


bool virtual_interface_test(){
	return true;
}