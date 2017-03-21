message("* qt/emuutils")


set(s_qt_emuutils_srcs
	  ../../common.cpp
	  ../../fifo.cpp
	  ../../fileio.cpp
)

if(WIN32)
include (GenerateExportHeader)

#add_library(emu_utils
#	  ${s_qt_emuutils_srcs}
#)
add_library(CSPemu_utils SHARED
	  ${s_qt_emuutils_srcs}
)

target_link_libraries(CSPemu_utils PUBLIC
		Qt5::Core
		-lz
	  )
set_target_properties(CSPemu_utils PROPERTIES
			     SOVERSION 2.4.0
			     VERSION 2.4.0
			     LINK_INTERFACE_LIBRARIES ""
			     )
#QT5_USE_MODULES(CSPgui Widgets Core Gui OpenGL)
#
generate_export_header(CSPemu_utils
        BASE_NAME CSPemu_utils
        EXPORT_MACRO_NAME CSPemu_utils_EXPORT
        EXPORT_FILE_NAME CSPemu_utils_Export.h
        STATIC_DEFINE CSPemu_utils_BUILT_AS_STATIC
)

else()
add_library(CSPemu_utils SHARED
	  ${s_qt_emuutils_srcs}
)

set_target_properties(CSPemu_utils PROPERTIES 
			     SOVERSION 2.4.0
			     VERSION 2.4.0
			     )
INSTALL(TARGETS CSPemu_utils DESTINATION ${LIBCSP_INSTALL_DIR})
endif()

