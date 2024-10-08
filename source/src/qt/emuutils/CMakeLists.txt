message("* qt/emuutils")

SET(THIS_LIB_VERSION 11.1.4)

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

MAKE_WRAP_CPP(s_qt_emuutils_headers_MOC ${s_qt_emuutils_headers})

add_compile_definitions(__LIBEMU_UTIL_VERSION=\"libCSPemu_utils.${THIS_LIB_VERSION}\")

if(WIN32)
	include (GenerateExportHeader)
	add_library(CSPemu_utils SHARED
		${s_qt_emuutils_srcs}
		${s_qt_emuutils_headers_MOC}
	)

	if(CHECK_QT_6)
		target_link_libraries(CSPemu_utils PRIVATE
			Qt::Core
			${ZLIB_LIBRARIES}
			${SDL2_LIBRARIES}
			${ADDITIONAL_DLL_LIBRARIES}
		)
	else()
		target_link_libraries(CSPemu_utils PRIVATE
		Qt5::Core
		${ZLIB_LIBRARIES}
		${SDL2_LIBRARIES}
		${ADDITIONAL_DLL_LIBRARIES}
		)
	endif()
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
