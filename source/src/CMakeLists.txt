message("* common/common")

#set(CMAKE_BUILD_SETTING_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -msse -mmmx")

add_library(common_emu
	emu.cpp
)
if(USE_DEVICES_SHARED_LIB)
  add_library(common_common
	config.cpp
	debugger.cpp
#	statesub.cpp
	)
else()
  add_library(common_common
	common.cpp
	config.cpp
	debugger.cpp
	fileio.cpp
	fifo.cpp
	statesub.cpp
	)
endif()
