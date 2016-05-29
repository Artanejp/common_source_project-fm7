message("* qt/osd")
set(s_qt_osd_headers
	  osd_base.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_osd_headers_MOC ${s_qt_osd_headers})
else()
  QT4_WRAP_CPP(s_qt_osd_headers_MOC ${s_qt_osd_headers})
endif()
#if(USE_SOCKET)
#  set(QT_COMMON_BASE ${QT_COMMON_BASE} qt_socket.cpp)
#endif()

 
SET(s_qt_osd_srcs
	  osd_base.cpp
	  osd_screen.cpp
	  osd_console.cpp
	  osd_input.cpp
	  osd_printer.cpp
	  osd_socket.cpp
	  osd_sound.cpp
	  osd_video.cpp
	  ${s_qt_osd_headers_MOC}
)

if(WIN32)
  include (GenerateExportHeader)

  add_library(qt_osd
    ${s_qt_osd_srcs}
    ${s_qt_osd_headers_MOC}
  )

else()
  add_library(CSPosd SHARED
    ${s_qt_osd_srcs}
    ${s_qt_osd_headers_MOC}
  )
  
  set_target_properties(CSPosd PROPERTIES 
     SOVERSION 1.1.2
     VERSION 1.1.2
  )
  INSTALL(TARGETS CSPosd DESTINATION ${LIBCSP_INSTALL_DIR})
endif()

