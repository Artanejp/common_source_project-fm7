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
add_library(qt_osd
    ${s_qt_osd_srcs}
    ${s_qt_osd_headers_MOC}
    )
#include (GenerateExportHeader)
#  add_library(CSPosd SHARED
#    ${s_qt_osd_srcs}
#    ${s_qt_osd_headers_MOC}
#  )
#target_link_libraries(CSPosd PUBLIC
#	   ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPemu_utils/build-win32/qt/emuutils/libemu_utils.a
#	   ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPavio/build-win32/qt/avio/libqt_avio.a
#	   ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPosd/build-win32/qt/osd/libqt_osd.a
#	  Qt5::Core
#	  Qt5::Gui
#	  Qt5::OpenGL
#	  Qt5::Widgets
#	  ${SDL2_LIBRARIES}
#	  )
#generate_export_header(CSPosd
#       BASE_NAME CSPosd
#       EXPORT_MACRO_NAME CSPosd_EXPORT
#       EXPORT_FILE_NAME CSPosd_Export.h
#       STATIC_DEFINE CSPgosd_BUILT_AS_STATIC
#)

else()
  add_library(CSPosd SHARED
    ${s_qt_osd_srcs}
    ${s_qt_osd_headers_MOC}
  )
target_link_libraries(CSPosd PUBLIC
	  Qt5::Core
	  Qt5::Gui
	  Qt5::Widgets
	  ${SDL2_LIBRARIES}
	  )
  
  set_target_properties(CSPosd PROPERTIES 
     SOVERSION 2.0.1
     VERSION 2.0.1
  )
  INSTALL(TARGETS CSPosd DESTINATION ${LIBCSP_INSTALL_DIR})
endif()

