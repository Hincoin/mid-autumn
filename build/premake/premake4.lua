
local root_dir="../.."
local os_type=os.get()--windows,linux,bsd,macosx

solution "mid-autumn"
configurations {"Debug","Release"}

--simple setup
configuration "Debug"
	objdir (root_dir .. "/build/obj/debug/")
	defines {"_DEBUG","DEBUG"}
	flags {"Symbols"}
	targetdir (root_dir .. "/build/bin/debug")
	libdirs (root_dir .. "/build/bin/debug")
	
configuration "Release"
	objdir (root_dir .. "/build/obj/release/")
	defines {"NDEBUG"}
	flags {"Symbols","Optimize","StaticRuntime","FloatFast"}
	targetdir (root_dir .. "/build/bin/release")
	libdirs(root_dir .. "/build/bin/release")

configuration {"gmake"}
buildoptions{"-fno-strict-aliasing",--[["-std=c++0x",]]--[["-fpermissive",]]"-g","-pg",--[["-fexcess-precision=fast",]]"-Wall"}
linkoptions{"-pg"}
defines{"_GNU_C_"}
if os.is("windows") then
defines{"WIN32","_WIN32","_WIN32_"}
end
--defines{"_GLIBCXX_PROFILE"}
configuration{"gmake","win32"}
defines{"WIN32","_WIN32"}
links{"wsock32","ws2_32"--[[,"pbr"]]}

location (root_dir .. "/build/premake/projects")
--------------------------------------------------------------------
project "lua"
kind "StaticLib"
language "C"

includedirs{
	root_dir .. "/extern/lua",
}
files{
	root_dir .. "/extern/lua/*.h",
	root_dir .. "/extern/lua/*.c",
}

--------------------------------------------------------------------
project "core"
kind "ConsoleApp"
language "C++"

includedirs{
	root_dir .. "/extern/",
	root_dir .. "/inc/core/",
}

files{ 
	root_dir .. "/src/core/*.hpp",
	root_dir .. "/src/core/*.cpp",
}
	defines{"CORE_SOURCE"}

----------------------------------------------------------------------
--a static core library
project "pbr"
kind "StaticLib"
language "C++"

includedirs{
	root_dir .. "/extern/",
	root_dir .. "/extern/tbb/include/",
	root_dir .. "/inc/core/",
	root_dir .. "/inc/pbr/",
	root_dir .. "/inc/pbr_net/",
}

files{ 
	root_dir .. "/inc/pbr/*.hpp",
	root_dir .. "/src/pbr/*.cpp",
}
defines{"CORE_SOURCE"}
	
-----------------------------------------------------------------------
--an application with extensions
project "pbr_ext"
kind "StaticLib"
language "C++"

includedirs{
	root_dir .. "/extern/",
	root_dir .. "/extern/tbb/include/",
	root_dir .. "/extern/oolua/include/",
	root_dir .. "/extern/lua/",
	root_dir .. "/inc/core/",
	root_dir .. "/inc/pbr/",
	root_dir .. "/inc/pbr_ext/",
	root_dir .. "/inc/pbr_net/",
}

files{ 
	root_dir .. "/inc/pbr/*.hpp",
	root_dir .. "/inc/pbr_ext/*.hpp",
	root_dir .. "/src/pbr_ext/*.cpp",
}

configuration{"Debug"}
	links{"pbr","lua","oolua_d" ,"pthread"--[[,"pbr"]]}
configuration{"Release"}
	links{"pbr","lua","oolua", "pthread"}

	
---------------------------------------------------------------------
project "pbr_svr"
kind "ConsoleApp"
language "C++"

includedirs{
	root_dir .. "/extern/",
	root_dir .. "/extern/oolua/include/",

	root_dir .. "/extern/lua/",
	root_dir .. "/inc/core/",
	root_dir .. "/inc/pbr/",
	root_dir .. "/inc/pbr_net/",
	root_dir .. "/inc/pbr_svr/",
	root_dir .. "/inc/pbr_ext/",
}

files{ 
	root_dir .. "/inc/core/*.hpp",
	root_dir .. "/inc/pbr_net/*.hpp",
	root_dir .. "/src/pbr_net/*.cpp",
	root_dir .. "/src/pbr_svr/*.cpp",
}
if os.is("windows") then
links{"wsock32","ws2_32"}
end

configuration{"Debug"}
	links{"pbr","pbr_ext","oolua_d" ,"lua","pthread"--[[,"pbr"]]}
configuration{"Release"}
	links{"pbr","pbr_ext","oolua" ,"lua","pthread"--[[,"pbr"]]}

project "pbr_clnt"
kind "ConsoleApp"
language "C++"

includedirs{
	root_dir .. "/extern/",
	root_dir .. "/extern/oolua/include/",

	root_dir .. "/extern/lua/",
	root_dir .. "/inc/core/",
	root_dir .. "/inc/pbr/",
	root_dir .. "/inc/pbr_net/",
	root_dir .. "/inc/pbr_clnt/",
	root_dir .. "/inc/pbr_ext/",
}

files{ 
	root_dir .. "/inc/core/*.hpp",
	root_dir .. "/inc/pbr_net/*.hpp",
	root_dir .. "/src/pbr_net/*.cpp",
	root_dir .. "/src/pbr_clnt/*.cpp",
}


if os.is("windows") then
links{"wsock32","ws2_32"}
end

configuration{"Debug"}
	links{"pbr_ext","pbr","oolua_d","lua" ,"pthread",--[[,"pbr"]]}
configuration{"Release"}
	links{"pbr","pbr_ext","oolua","lua" ,"pthread",--[[,"pbr"]]}

project "pbr_ctrl"
kind "ConsoleApp"
language "C++"

includedirs{
	root_dir .. "/extern/",
	root_dir .. "/extern/oolua/include/",

	root_dir .. "/extern/lua/",
	root_dir .. "/inc/core/",
	root_dir .. "/inc/pbr/",
	root_dir .. "/inc/pbr_net/",
	root_dir .. "/inc/pbr_ext/",
	root_dir .. "/inc/pbr_ctrl/",
}

files{ 
	root_dir .. "/inc/core/*.hpp",
	root_dir .. "/src/pbr_net/*.cpp",
	root_dir .. "/inc/pbr_ctrl/*.hpp",
	root_dir .. "/src/pbr_ctrl/*.cpp",
}

if os.is("windows") then
links{"wsock32","ws2_32"}
end

configuration{"Debug"}
	links{"oolua_d" ,"lua","pthread"--[[,"pbr"]]}
configuration{"Release"}
	links{"oolua" ,"lua","pthread"--[[,"pbr"]]}



-----------------------------------------------------------------------
cwd = os.getcwd()
cp_cmd="cp"
mk_cmd="make"
if os_type == "windows" then
	cp_cmd="copy"
	mk_cmd="mingw32-make"--use mingw by default
end
--external libraries
--tbb
os.chdir((root_dir .. "/extern/tbb"))
tbb_dir = os.getcwd()
--lua
os.chdir(cwd)
os.chdir(root_dir .. "/extern/lua")
lua_dir = os.getcwd()

--gmock
os.chdir(cwd)

--gmock
--
--cppunit
--
--oolua
--
-----------------------------------------------------------------------
