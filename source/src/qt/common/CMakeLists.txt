message("* qt/common")
set(s_qt_common_headers
	  emu_thread.h
	  mainwidget.h
	  ../osd.h
)
if(USE_SOCKET)
  set(s_qt_common_headers ${s_qt_common_headers} ../osd_socket.h)
endif()

MAKE_WRAP_CPP(s_qt_common_headers_MOC ${s_qt_common_headers})

set(QT_COMMON_BASE
#      main.cpp 
      qt_utils.cpp
	  menu_flags.cpp
	  
	  emu_thread.cpp
#	  emu_thread_slots.cpp
##	  util_fd2.cpp
#	  util_hdd2.cpp
	  util_bubble2.cpp
	  util_main.cpp
	  ../osd.cpp
	  ../osd_wrapper.cpp
)

if(WIN32)
	set(QT_COMMON_BASE ../gui/qt_main.cpp ${QT_COMMON_BASE} )

  add_executable(${EXEC_TARGET}
	  WIN32
	  ${QT_COMMON_BASE}
	  ${s_qt_common_headers_MOC}
	  )
 else()
	add_executable(${EXEC_TARGET}
	  ${QT_COMMON_BASE}
	  ${s_qt_common_headers_MOC}
	  )
endif()
#cotire(${EXEC_TARGET})

if(CHECK_QT_6)
  if(USE_SOCKET)
    QT_USE_MODULES(${EXEC_TARGET} Widgets Core Gui OpenGL Network Qt5CoreCompat)
  else()
    QT_USE_MODULES(${EXEC_TARGET} Widgets Core Gui OpenGL Qt5CoreCompat)
  endif()
else()
  if(USE_SOCKET)
    QT5_USE_MODULES(${EXEC_TARGET} Widgets Core Gui OpenGL Network)
  else()
    QT5_USE_MODULES(${EXEC_TARGET} Widgets Core Gui OpenGL)
  endif()
endif()


if(WIN32)
  if(CHECK_QT_6)
	find_package(Qt6Core)
  else()
	find_package(Qt5Core)
  endif()
  target_link_libraries(${EXEC_TARGET} ${LOCAL_LIBS} ${BUNDLE_LIBS} -lpthread)
else()
  target_link_libraries(${EXEC_TARGET} ${LOCAL_LIBS} ${BUNDLE_LIBS} -lpthread)
endif()

install(TARGETS ${EXEC_TARGET} DESTINATION bin)

