message("* agar/common")

#set(CMAKE_BUILD_SETTING_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -msse -mmmx")
 
add_library(common_common
	  common.cpp
	  config.cpp
	  emu.cpp
	  fifo.cpp
	  fileio.cpp
)

