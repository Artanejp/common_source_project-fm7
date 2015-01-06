message("* agar/common")

#set(CMAKE_BUILD_SETTING_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -msse -mmmx")
add_library(agar_common
#	  agar_glcl.cpp
#	  agar_gldraw2.cpp
#	  agar_glutil.cpp
	  agar_logger.cpp
	  agar_sdlscaler.cpp
	  agar_wrapper.cpp
	  sdl_sound.cpp
	  agar_main.cpp
	  agar_emuevents.cpp
	  agar_sdlview.c
	  sdl_cpuid.c
)
