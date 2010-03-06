#!/bin/bash
./premake4 gmake linux
cd projects
make config=$1
cd ..
