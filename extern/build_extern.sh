#!/bin/bash

#simple build script
cd tbb
make
cd ..
#cd cppunit
#./configure
#make
#cd ..

#cd gmock/make
#make
#cp *.a ../../../build/bin/debug
#cp *.so ../../../build/bin/debug
#cp *.a ../../../build/bin/release
#cp *.so ../../../build/bin/release
#cd ../../

cd oolua
./premake4 gmake linux
make oolua
cp bin/Debug/* ../../build/bin/debug
cp bin/Release/* ../../build/bin/release
cd ..

