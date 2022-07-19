message("* qt/osd")
SET(THIS_LIB_VERSION 7.0.0)

set(s_qt_osd_headers
	osd_base.h
	osd_socket.h
	
    sound_buffer_qt.h
)

MAKE_WRAP_CPP(s_qt_osd_headers_MOC ${s_qt_osd_headers})

SET(s_qt_osd_srcs
	osd_base.cpp
	osd_screen.cpp
	osd_console.cpp
	osd_input.cpp
	osd_printer.cpp
	osd_socket.cpp
	osd_sound.cpp
	osd_video.cpp
	qudpsocket2.cpp
	qtcpsocket2.cpp
	sound_buffer_qt.cpp
	
	${s_qt_osd_headers_MOC}
)
add_definitions(-D__LIBOSD_VERSION=\"libCSPosd.${THIS_LIB_VERSION}\")

include (GenerateExportHeader)

add_library(CSPosd SHARED
	${s_qt_osd_srcs}
	${s_qt_osd_headers_MOC}
)
set_std(CSPosd)


if(WIN32)
  if(USE_QT_6)
	target_link_libraries(CSPosd PUBLIC
	  CSPemu_utils
	  Qt::Core
	  Qt::Gui
	  Qt::OpenGL
	  Qt::Widgets
	  Qt::Network
	  Qt::Multimedia
	  ${SDL2_LIBRARIES}
	  ${ADDITIONAL_DLL_LIBRARIES}
	  )
  elseif(USE_QT_5)
	target_link_libraries(CSPosd
	  PUBLIC CSPemu_utils
	  Qt5::Core
	  Qt5::Gui
	  Qt5::OpenGL
	  Qt5::Widgets
	  Qt5::Network
	  Qt5::Multimedia
	  ${SDL2_LIBRARIES}
	  ${ADDITIONAL_DLL_LIBRARIES}
	  )
  endif()
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
  if(USE_QT_6)
	target_link_libraries(CSPosd PUBLIC
		Qt::Core
		Qt::Gui
		Qt::Widgets
		Qt::Network
		Qt::Multimedia
		${SDL2_LIBRARIES}
		${ADDITIONAL_DLL_LIBRARIES}
		)
	elseif(USE_QT_5)
	target_link_libraries(CSPosd PUBLIC
		Qt5::Core
		Qt5::Gui
		Qt5::Widgets
		Qt5::Network
		Qt5::Multimedia
		${SDL2_LIBRARIES}
		${ADDITIONAL_DLL_LIBRARIES}
		)
	endif()
	set_target_properties(CSPosd PROPERTIES 
	  SOVERSION ${THIS_LIB_VERSION}
	  VERSION ${THIS_LIB_VERSION}
	  )
	INSTALL(TARGETS CSPosd DESTINATION ${LIBCSP_INSTALL_DIR})
endif()


