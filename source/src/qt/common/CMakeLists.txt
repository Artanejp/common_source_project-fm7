message("* qt/common")

 
add_library(qt_common
	  emuevents_control.cpp
	  qt_gldraw.cpp
	  qt_main.cpp
	  qt_screen.cpp
	  agar_logger.cpp
	  sdl_sound.cpp
	  sdl_cpuid.c
)

