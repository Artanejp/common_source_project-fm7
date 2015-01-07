message("* agar/ui")

#set(CMAKE_BUILD_SETTING_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -msse -mmmx")
 
add_library(agar_ui
	  agar_dialogs.cpp
)
