message("* agar/menu")

#set(CMAKE_BUILD_SETTING_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -msse -mmmx")
 
add_library(agar_menu
	  agar_menu.cpp
	  menu_screen.cpp
)
