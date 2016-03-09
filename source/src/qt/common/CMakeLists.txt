message("* qt/common")

set(s_qt_common_headers
	  qt_dialogs.h
	  qt_gldraw.h
	  qt_glutil_gl2_0.h
	  qt_glutil_gl3_0.h
	  emu_thread.h
	  draw_thread.h
)

if(WITH_JOYSTICK)
  set(QT_COMMON_JOYSTICK_SRC joy_thread.cpp)
  set(s_qt_common_headers ${s_qt_common_headers} joy_thread.h)
endif()

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_common_headers_MOC ${s_qt_common_headers})
else()
  QT4_WRAP_CPP(s_qt_common_headers_MOC ${s_qt_common_headers})
endif()

set(QT_COMMON_BASE  emuevents_control.cpp
	  qt_gldraw.cpp
	  qt_glutil.cpp
	  qt_glutil_gl2_0.cpp
	  qt_glutil_gl3_0.cpp
	  qt_glevents.cpp
	  qt_dialogs.cpp
	  qt_main.cpp
	  emu_thread.cpp
	  draw_thread.cpp
	  agar_logger.cpp
)

if(USE_QT_5)
  QT5_ADD_RESOURCES(s_qt_common_RCC ${RESOURCE}
                    ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/commontexts.qrc
                    ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/shaders.qrc
                    ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/icons.qrc)
else()
  QT4_ADD_RESOURCES(s_qt_common_RCC ${RESOURCE} 
                    ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/commontexts.qrc
                    ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/shaders.qrc
                    ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/icons.qrc)
endif()

add_executable(${EXEC_TARGET}
	  ${QT_COMMON_BASE}
	  ${QT_COMMON_JOYSTICK_SRC}
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

