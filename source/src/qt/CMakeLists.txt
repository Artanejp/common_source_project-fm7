message("* qt/osd")
set(s_qt_osd_headers
	osd_base.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_osd_headers_MOC ${s_qt_osd_headers})
else()
  QT4_WRAP_CPP(s_qt_osd_headers_MOC ${s_qt_osd_headers})
endif()

SET(THIS_LIB_VERSION 2.9.7) 
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
add_definitions(-D_LIBOSD_VERSION=${THIS_LIB_VERSION})

if(WIN32)
#add_library(qt_osd
#    ${s_qt_osd_srcs}
#    ${s_qt_osd_headers_MOC}
#    )
#set_property(TARGET qt_osd PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
include (GenerateExportHeader)
  add_library(CSPosd SHARED
    ${s_qt_osd_srcs}
    ${s_qt_osd_headers_MOC}
  )
target_link_libraries(CSPosd PUBLIC
          ${CMAKE_SOURCE_DIR}/../../build-cmake/bin-win32/libCSPemu_utils.dll.a
#          ${CMAKE_SOURCE_DIR}/../../build-cmake/bin-win32/libqt_avio.a
	  Qt5::Core
	  Qt5::Gui
	  Qt5::OpenGL
	  Qt5::Widgets
	  ${SDL2_LIBRARIES}
	  )
generate_export_header(CSPosd
       BASE_NAME CSPosd
       EXPORT_MACRO_NAME CSPosd_EXPORT
       EXPORT_FILE_NAME CSPosd_Export.h
       STATIC_DEFINE CSPgosd_BUILT_AS_STATIC
)
  set_target_properties(CSPosd PROPERTIES 
     SOVERSION ${THIS_LIB_VERSION}
     VERSION ${THIS_LIB_VERSION}
  )

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
     SOVERSION ${THIS_LIB_VERSION}
     VERSION ${THIS_LIB_VERSION}
  )
  INSTALL(TARGETS CSPosd DESTINATION ${LIBCSP_INSTALL_DIR})
endif()

