


premake4 --file=../../extern/oolua/premake4.lua gmake

cd ../../extern/oolua/
mingw32-make
copy bin/Debug/* ../../build/bin/debug
copy bin/Release/* ../../build/bin/release

cd ../../build/premake
