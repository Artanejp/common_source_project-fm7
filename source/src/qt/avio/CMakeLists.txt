message("* qt/avio")

set(s_qt_avio_headers
	  movie_saver.h
)

set(s_qt_avio_srcs
	  movie_saver.cpp
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_avio_headers_MOC ${s_qt_avio_headers})
else()
  QT4_WRAP_CPP(s_qt_avio_headers_MOC ${s_qt_avio_headers})
endif()

if(WIN32)
include (GenerateExportHeader)

add_library(qt_avio
	  ${s_qt_avio_srcs}
	  ${s_qt_avio_headers_MOC}
)

#add_library(CSPgui SHARED
#	  ${s_qt_avio_srcs}
#	  ${s_qt_avio_headers_MOC}
#	  )
#target_link_libraries(CSPavio PUBLIC
#${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPavio/build-win32/qt/avio/CSPavio.dll
#	  Qt5::Core
#	  Qt5::Gui
#	  Qt5::OpenGL
#	  Qt5::Svg
#	  Qt5::Widgets
#	  )
#QT5_USE_MODULES(CSPavio Widgets Core Gui OpenGL)
#
#generate_export_header(CSPavio
#        BASE_NAME CSPavio
#        EXPORT_MACRO_NAME CSPavio_EXPORT
#        EXPORT_FILE_NAME CSPavio_Export.h
#        STATIC_DEFINE CSPavio_BUILT_AS_STATIC
#)

else()
add_library(CSPavio SHARED
	  ${s_qt_avio_srcs}
	  ${s_qt_avio_headers_MOC}
)

set_target_properties(CSPavio PROPERTIES 
			     SOVERSION 1.0.0
			     VERSION 1.0.0
			     )
INSTALL(TARGETS CSPavio DESTINATION ${LIBCSP_INSTALL_DIR})
endif()

