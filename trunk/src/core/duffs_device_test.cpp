

#include "DuffsDevice.hpp"
#include "simple_test_framework.hpp"

//using namespace ma;
inline bool duffs_device_test(){

	int i = 0;
	DUFFS_DEVICE(16,int,0, i++; );
	bool result = (i == 0);
	DUFFS_DEVICE(128,int , 1000, i+=2;);
	result = result && (i== 2000);
	return result;
}

REGISTER_TEST_FUNC(duffs_device,duffs_device_test)