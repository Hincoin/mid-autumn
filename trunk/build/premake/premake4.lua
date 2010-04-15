
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
	flags {"Optimize","StaticRuntime","FloatFast"}
	targetdir (root_dir .. "/build/bin/release")
	libdirs(root_dir .. "/build/bin/release")

configuration {"gmake"}
buildoptions{"-std=c++0x","-fpermissive","-ggdb","-pg"}
defines{"_GNU_C_"}

configuration{"win32"}
defines{"WIN32","_WIN32"}

location (root_dir .. "/build/premake/projects")
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
}

files{ 
	root_dir .. "/inc/pbr/*.hpp",
	root_dir .. "/src/pbr/*.cpp",
}
defines{"CORE_SOURCE"}
	
-----------------------------------------------------------------------
--an application with extensions
project "pbr_ext"
kind "ConsoleApp"
language "C++"

includedirs{
	root_dir .. "/extern/",
	root_dir .. "/extern/tbb/include/",
	root_dir .. "/extern/oolua/include/",
	root_dir .. "/inc/core/",
	root_dir .. "/inc/pbr/",
	root_dir .. "/inc/pbr_ext/",
}

files{ 
	root_dir .. "/inc/pbr/*.hpp",
	root_dir .. "/inc/pbr_ext/*.hpp",
	root_dir .. "/src/pbr_ext/*.cpp",
}

configuration{"Debug"}
	links{"pbr","lua","oolua_d" --[[,"pbr"]]}
configuration{"Release"}
	links{"pbr","lua","oolua"}

	
	
	
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