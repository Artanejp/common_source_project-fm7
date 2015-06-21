message("* qt/common")

set(s_qt_common_headers
	  qt_dialogs.h
	  qt_gldraw.h
	  qt_main.h
)

QT4_WRAP_CPP(s_qt_common_headers_MOC ${s_qt_common_headers})

set(QT_COMMON_BASE  emuevents_control.cpp
	  emu_utils.cpp
	  qt_gldraw.cpp
	  qt_glutil.cpp
	  qt_glevents.cpp
	  qt_dialogs.cpp
	  qt_main.cpp
	  qt_screen.cpp
	  agar_logger.cpp
	  sdl_sound.cpp
	  emu_input.cpp
	  sdl_cpuid.c
)

if(USE_SOCKET)
  set(QT_COMMON_BASE ${QT_COMMON_BASE} qt_socket.cpp)
endif()

add_executable(${EXEC_TARGET}
	  ${QT_COMMON_BASE}
	  ${s_qt_common_headers_MOC}
)

target_link_libraries(${EXEC_TARGET} ${LOCAL_LIBS} ${BUNDLE_LIBS})

install(TARGETS ${EXEC_TARGET} DESTINATION bin)

