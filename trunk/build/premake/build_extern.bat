


premake4 --file=../../extern/oolua/premake4.lua gmake

cd ../../extern/oolua/
mingw32-make oolua
mingw32-make config=release oolua
copy /y "bin\Debug\*.*" "..\..\build\bin\debug\"
copy /y "bin\Release\*.*" "..\..\build\bin\release\"

cd ../../build/premake
