message("* qt/common")

set(s_qt_common_headers
	  emu_thread.h
	  mainwidget.h
	  ../osd.h
)
if(USE_SOCKET)
  set(s_qt_common_headers ${s_qt_common_headers} ../osd_socket.h)
endif()

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_common_headers_MOC ${s_qt_common_headers})
else()
  QT4_WRAP_CPP(s_qt_common_headers_MOC ${s_qt_common_headers})
endif()

set(QT_COMMON_BASE  
	  qt_utils.cpp
	  menu_flags.cpp
	  
	  emu_thread.cpp
	  emu_thread_slots.cpp
	  util_fd2.cpp
	  util_bubble2.cpp
	  util_main.cpp
	  ../osd.cpp
	  ../osd_wrapper.cpp
)


#if(WIN32)
  QT5_ADD_RESOURCES(s_qt_common_RCC ${RESOURCE}
		      ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/i18n_global.qrc
		      ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/icons.qrc)
#else()
#  QT5_ADD_RESOURCES(s_qt_common_RCC ${RESOURCE}
#                    ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/icons.qrc)
#endif()
#include(cotire)

add_executable(${EXEC_TARGET}
	  ${QT_COMMON_BASE}
	  ${s_qt_common_headers_MOC}
	  ${s_qt_common_RCC}
)
#cotire(${EXEC_TARGET})

if(USE_QT_5)
  if(USE_SOCKET)
    QT5_USE_MODULES(${EXEC_TARGET} Widgets Core Gui OpenGL Network)
  else()
    QT5_USE_MODULES(${EXEC_TARGET} Widgets Core Gui OpenGL)
  endif()
endif()
if(WIN32)
 if(USE_FMGEN)
   set(BUNDLE_LIBS
	 ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPfmgen/build-win32/vm/fmgen/libvm_fmgen.a
	 ${BUNDLE_LIBS}
   )
 endif()

 target_link_libraries(${EXEC_TARGET} 
 ${LOCAL_LIBS}
 ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPemu_utils/build-win32/qt/emuutils/libemu_utils.a
 ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPosd/build-win32/qt/osd/libqt_osd.a
 ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPgui/build-win32/qt/gui/libqt_gui.a
 ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPavio/build-win32/qt/avio/libqt_avio.a
 ${BUNDLE_LIBS} 
 ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPcommon_vm/build-win32/vm/common_vm/libvm_common_vm.a
 ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPgui/build-win32/qt/gui/libqt_gui.a
 Qt5::OpenGL
 )
else()
 target_link_libraries(${EXEC_TARGET} ${LOCAL_LIBS} ${BUNDLE_LIBS})
endif()
install(TARGETS ${EXEC_TARGET} DESTINATION bin)

