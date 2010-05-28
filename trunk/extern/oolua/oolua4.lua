--
-- OOLua binding premake script
--
create_package("oolua","./","StaticLib")

configuration {}

files { "include/*.h", "src/*.cpp" }
includedirs 
					{
							"../",
							"../lua/",
						"include/lua/",
						"include/",
						"/usr/local/include",
						"/usr/include",
				} 
					
defines {"OOLUA_STORE_ERROR"}
configuration { "gmake" }
  buildoptions { "-std=c++0x"}
