message("* qt/emuutils")


set(s_qt_emuutils_srcs
	  ../../common.cpp
	  ../../fifo.cpp
	  ../../fileio.cpp
)

if(WIN32)
include (GenerateExportHeader)

add_library(emu_utils
	  ${s_qt_emuutils_srcs}
)


#add_library(CSPgui SHARED
#	  ${s_qt_gui_srcs}
#	  ${s_qt_gui_headers_MOC}
#	  )
#target_link_libraries(CSPgui PUBLIC
#	   ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPosd/build-win32/qt/osd/CSPosd.dll
#	  Qt5::Core
#	  Qt5::Gui
#	  Qt5::OpenGL
#	  Qt5::Svg
#	  Qt5::Widgets
#	  )
#QT5_USE_MODULES(CSPgui Widgets Core Gui OpenGL)
#
#generate_export_header(CSPgui
#        BASE_NAME CSPgui
#        EXPORT_MACRO_NAME CSPgui_EXPORT
#        EXPORT_FILE_NAME CSPgui_Export.h
#        STATIC_DEFINE CSPgui_BUILT_AS_STATIC
#)

else()
add_library(CSPemu_utils SHARED
	  ${s_qt_emuutils_srcs}
)

set_target_properties(CSPemu_utils PROPERTIES 
			     SOVERSION 1.0.1
			     VERSION 1.0.1
			     )
INSTALL(TARGETS CSPemu_utils DESTINATION ${LIBCSP_INSTALL_DIR})
endif()

