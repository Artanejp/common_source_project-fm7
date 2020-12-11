message("* qt/emuutils")

SET(THIS_LIB_VERSION 2.22.1)

set(s_qt_emuutils_headers
	../gui/csp_logger.h
	)
	
set(s_qt_emuutils_srcs
	  ../../common.cpp
	  ../../config_dll.cpp
	  ../../fifo.cpp
	  ../../fileio.cpp
	  ../../ringbuffer.cpp
	  ../gui/csp_logger.cpp
	  )

QT5_WRAP_CPP(s_qt_emuutils_headers_MOC ${s_qt_emuutils_headers})
	

add_definitions(-D__LIBEMU_UTIL_VERSION=\"libCSPemu_utils.${THIS_LIB_VERSION}\")

if(WIN32)

include (GenerateExportHeader)

add_library(CSPemu_utils SHARED
	  ${s_qt_emuutils_srcs}
	  ${s_qt_emuutils_headers_MOC}
)

target_link_libraries(CSPemu_utils PRIVATE
		Qt5::Core
		${ZLIB_LIBRARIES}
		${SDL2_LIBRARIES}
	  )
	  
set_target_properties(CSPemu_utils PROPERTIES
			     SOVERSION ${THIS_LIB_VERSION}
			     VERSION ${THIS_LIB_VERSION}
			     LINK_INTERFACE_LIBRARIES ""
			     )
generate_export_header(CSPemu_utils
        BASE_NAME CSPemu_utils
        EXPORT_MACRO_NAME CSPemu_utils_EXPORT
        EXPORT_FILE_NAME CSPemu_utils_Export.h
        STATIC_DEFINE CSPemu_utils_BUILT_AS_STATIC
)
	set_std(CSPemu_utils)

else()
add_library(CSPemu_utils SHARED
	  ${s_qt_emuutils_srcs}
	  ${s_qt_emuutils_headers_MOC}
)

set_target_properties(CSPemu_utils PROPERTIES 
			     SOVERSION ${THIS_LIB_VERSION}
			     VERSION ${THIS_LIB_VERSION}
			     )
	set_std(CSPemu_utils)

INSTALL(TARGETS CSPemu_utils DESTINATION ${LIBCSP_INSTALL_DIR})

endif()
