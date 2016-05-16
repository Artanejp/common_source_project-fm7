message("* qt/common")

set(s_qt_common_headers
	  emu_thread.h
	  menu_metaclass.h
	  mainwidget.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_common_headers_MOC ${s_qt_common_headers})
else()
  QT4_WRAP_CPP(s_qt_common_headers_MOC ${s_qt_common_headers})
endif()

set(QT_COMMON_BASE  
	  qt_utils.cpp
	  menu_flags.cpp
	  menu_metaclass.cpp
	  emu_thread.cpp
	  util_fd2.cpp
	  util_bubble2.cpp
	  util_main.cpp
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
if(WIN32)
 target_link_libraries(${EXEC_TARGET} 
 ${LOCAL_LIBS}
 ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPgui/build-win32/qt/gui/libqt_gui.a
 ${BUNDLE_LIBS} 
 ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPgui/build-win32/qt/gui/libqt_gui.a
 Qt5::OpenGL
 )
else()
 target_link_libraries(${EXEC_TARGET} ${LOCAL_LIBS} ${BUNDLE_LIBS})
endif()
install(TARGETS ${EXEC_TARGET} DESTINATION bin)

