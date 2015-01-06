message("* agar/common")

#set(CMAKE_BUILD_SETTING_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -msse -mmmx")
add_executable(${EXEC_TARGET}
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

target_link_libraries(${EXEC_TARGET} ${LOCAL_LIBS} ${BUNDLE_LIBS})

install(TARGETS ${EXEC_TARGET} DESTINATION bin)
