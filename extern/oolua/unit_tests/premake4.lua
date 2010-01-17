--OOLua test.unit
local root = "../"
local name = "test.unit"
create_package(name,root,"ConsoleApp")


configuration {}

files 
{ 
    	"**.h",
    	"**.cpp"
}
includedirs 
{
	"include/cppunit",
	"include/gmock",
--	"../" .. "../" .. "cppunit/include",
--	"../" .. "../" .."gmock/include",
	"include/lua",
	"include/",
	root .. "include/",
	"/usr/local/include",
	"/usr/include",
	"./bind_classes",
	"./cpp_classes",
	"./test_classes"
} 
					
defines 
{
	"USING_CPPUNIT",
	"USING_GMOCK",
	"OOLUA_STORE_ERROR"
}

links
{
	"oolua"
}

	configuration { "vs*"}
		postbuildcommands { "$(TargetPath)" }
		links{"lua51"}
		
	configuration { "vs*","Debug"}
		links{ "cppunitd" , "gmockd" }

	configuration { "vs*","Release"}
		links{ "cppunit" , "gmock" }
		
	configuration {"codeblocks" }
		postbuildcommands { "$(TARGET_OUTPUT_FILE)"}
				

	configuration {"gmake or codeblocks","linux or macosx" }
		libdirs {"usr/local/lib","usr/lib"}
		links{ "cppunit", "lua", "gmock" }
		linkoptions{"`../../gmock/scripts/gmock-config --cxxflags --ldflags --libs`"}


	configuration {"xcode3" }
		libdirs {"usr/local/lib","usr/lib"}
		links{ "gmock","gtest","cppunit", "lua" }
		postbuildcommands {"$TARGET_BUILD_DIR/$TARGET_NAME"}

	configuration {"windows","codeblocks","Debug" }
		links{ "lua", "cppunitd" , "gmockd" }
		
	configuration {"windows","codeblocks","Release" }	
		links{ "lua", "cppunit" , "gmock" }
		
	configuration {"gmake","Debug"}	
		postbuildcommands  { root .. "bin/Debug/" .. name }
	--	libdirs{"../../../build/bin/debug"}
		
	configuration {"gmake","Release"}	
	--	libdirs{"../../../build/bin/release"}
		postbuildcommands { root .. "bin/Release/" .. name }

	configuration {"linux" }
		links{ "dl" }

configuration { "linux", "gmake" }
  buildoptions { "-std=c++0x"}

