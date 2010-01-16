#!/bin/bash

#simple build script
cd tbb
make
cd ..
cd cppunit
#./configure
#make
cd ..
cd gmock/make
make
cd ../../
cd oolua
./premake4 gmake linux
make
cd ..

