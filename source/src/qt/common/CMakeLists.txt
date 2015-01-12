message("* qt/common")

set(s_qt_common_headers
#	  emu_utils.h
	  qt_dialogs.h
#	  qt_emuevents.h
	  qt_gldraw.h
#	  qt_input.h
)

QT4_WRAP_CPP(s_qt_common_headers_MOC ${s_qt_common_headers})

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
	  ${s_qt_common_headers_MOC}
)

target_link_libraries(${EXEC_TARGET} ${LOCAL_LIBS} ${BUNDLE_LIBS})

install(TARGETS ${EXEC_TARGET} DESTINATION bin)

