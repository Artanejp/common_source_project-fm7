message("* common/common")

#set(CMAKE_BUILD_SETTING_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -msse -mmmx")
 
add_library(common_common
	config.cpp
	debugger.cpp
#	romakana.cpp
)

add_library(common_emu
	emu.cpp
)
