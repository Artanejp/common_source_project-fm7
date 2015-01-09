message("* agar/pc8801MA")

#set(CMAKE_BUILD_SETTING_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -msse -mmmx")
 
add_library(agar_pc88ma
	  menu_pc88ma.cpp
)
