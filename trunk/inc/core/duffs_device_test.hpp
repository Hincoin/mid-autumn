#ifndef DUFFS_DEVICE_TEST_HPP_INCLUDED
#define DUFFS_DEVICE_TEST_HPP_INCLUDED


#include "DuffsDevice.hpp"


namespace ma_test{

    //using namespace ma;
    inline void duffs_device_test(){

        int i = 0;
        DUFFS_DEVICE(16,int,0, i++; );
        assert(i == 0);
        DUFFS_DEVICE(128,int , 1000, i++;);
        assert(i== 1000);
        }

}


#endif // DUFFS_DEVICE_TEST_HPP_INCLUDED
