message("* qt/common")

 
add_executable(${EXEC_TARGET}
	  emuevents_control.cpp
	  emu_utils.cpp
	  qt_gldraw.cpp
	  qt_glutil.cpp
	  qt_dialogs.cpp
	  qt_main.cpp
	  qt_screen.cpp
	  qt_input.cpp
	  agar_logger.cpp
	  sdl_sound.cpp
	  qt_debugger.cpp
	  sdl_cpuid.c
)
target_link_libraries(${EXEC_TARGET} ${LOCAL_LIBS} ${BUNDLE_LIBS})

install(TARGETS ${EXEC_TARGET} DESTINATION bin)

