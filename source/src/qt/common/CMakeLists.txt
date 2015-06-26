message("* qt/common")

set(s_qt_common_headers
	  qt_dialogs.h
	  qt_gldraw.h
	  qt_main.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_common_headers_MOC ${s_qt_common_headers})
else()
  QT4_WRAP_CPP(s_qt_common_headers_MOC ${s_qt_common_headers})
endif()

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

if(USE_QT_5)
  QT5_ADD_RESOURCES(s_qt_common_RCC ${RESOURCE})
else()
  QT4_ADD_RESOURCES(s_qt_common_RCC ${RESOURCE})
endif()

add_executable(${EXEC_TARGET}
	  ${QT_COMMON_BASE}
	  ${s_qt_common_headers_MOC}
	  ${s_qt_common_RCC}
)
if(USE_QT_5)
  if(USE_SOCKET)
    QT5_USE_MODULES(${EXEC_TARGET} Widgets Core Gui OpenGL Network)
  else()
    QT5_USE_MODULES(${EXEC_TARGET} Widgets Core Gui OpenGL)
  endif()
endif()
target_link_libraries(${EXEC_TARGET} ${LOCAL_LIBS} ${BUNDLE_LIBS})

install(TARGETS ${EXEC_TARGET} DESTINATION bin)

