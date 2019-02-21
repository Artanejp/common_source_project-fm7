message("* qt/common")

#include(cotire)
#include(PrecompiledHeader)

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
      main.cpp 
      qt_utils.cpp
	  menu_flags.cpp
	  
	  emu_thread.cpp
	  emu_thread_slots.cpp
#	  util_fd2.cpp
#	  util_hdd2.cpp
	  util_bubble2.cpp
	  util_main.cpp
	  ../osd.cpp
	  ../osd_wrapper.cpp
)


 QT5_ADD_RESOURCES(s_qt_common_RCC ${RESOURCE}
		      ${CMAKE_CURRENT_SOURCE_DIR}/qrc/i18n_global.qrc
		      ${CMAKE_CURRENT_SOURCE_DIR}/qrc/icons.qrc)

if(WIN32)
	set(QT_COMMON_BASE ../gui/qt_main.cpp ${QT_COMMON_BASE} )

  add_executable(${EXEC_TARGET}
	  WIN32
	  ${QT_COMMON_BASE}
	  ${s_qt_common_headers_MOC}
	  ${s_qt_common_RCC}
	  )
 else()
  add_executable(${EXEC_TARGET}
	  ${QT_COMMON_BASE}
	  ${s_qt_common_headers_MOC}
	  ${s_qt_common_RCC}
	  )
#  add_precompiled_header(${EXEC_TARGET} pch_list
#	  SOURCE_CXX pch.cpp FORCEINCLUDE
#	  )
endif()
#cotire(${EXEC_TARGET})

if(USE_QT_5)
  if(USE_SOCKET)
    QT5_USE_MODULES(${EXEC_TARGET} Widgets Core Gui OpenGL Network)
  else()
    QT5_USE_MODULES(${EXEC_TARGET} Widgets Core Gui OpenGL)
  endif()
endif()


if(WIN32)
 
find_package(Qt5Core)  
 target_link_libraries(${EXEC_TARGET}
 ${LOCAL_LIBS}
 ${CMAKE_SOURCE_DIR}/../../build-cmake/bin-win32/libCSPgui.dll.a
 ${CMAKE_SOURCE_DIR}/../../build-cmake/bin-win32/libCSPavio.dll.a

 #${CMAKE_SOURCE_DIR}/../../build-cmake/bin-win32/libCSPcommon_vm.dll.a
 ${CMAKE_SOURCE_DIR}/../../build-cmake/bin-win32/libvm_common_vm.a
 ${CMAKE_SOURCE_DIR}/../../build-cmake/bin-win32/libCSPfmgen.dll.a
 ${CMAKE_SOURCE_DIR}/../../build-cmake/bin-win32/libCSPosd.dll.a
 ${CMAKE_SOURCE_DIR}/../../build-cmake/bin-win32/libCSPemu_utils.dll.a
 ${BUNDLE_LIBS}
 -lpthread
 )
else()
 target_link_libraries(${EXEC_TARGET} ${LOCAL_LIBS} ${BUNDLE_LIBS} -lpthread)
endif()
install(TARGETS ${EXEC_TARGET} DESTINATION bin)

