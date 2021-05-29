message("* qt/avio")

SET(THIS_LIB_VERSION 3.2.3)
set(s_qt_avio_headers
	  movie_saver.h
	  movie_loader.h
	  sound_loader.h
)

set(s_qt_avio_srcs
	  movie_loader.cpp
	  movie_saver.cpp
	  movie_saver_audio.cpp
	  movie_saver_video.cpp
	  movie_saver_fileio.cpp
	  sound_loader.cpp
)
add_definitions(-D__LIBAVIO_VERSION=\"libCSPavio.${THIS_LIB_VERSION}\")

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_avio_headers_MOC ${s_qt_avio_headers})
else()
  QT4_WRAP_CPP(s_qt_avio_headers_MOC ${s_qt_avio_headers})
endif()

if(WIN32)
#add_library(qt_avio
#	  ${s_qt_avio_srcs}
#	  ${s_qt_avio_headers_MOC}
#)

include (GenerateExportHeader)
add_library(CSPavio SHARED
	  ${s_qt_avio_srcs}
	  ${s_qt_avio_headers_MOC}
)

target_link_libraries(CSPavio PUBLIC
		 PRIVATE ${LIBAV_LIBRARIES}
		 CSPosd
	  	 CSPemu_utils
		 Qt5::Core
		 Qt5::Gui
		 Qt5::Widgets
		 ${ADDITIONAL_DLL_LIBRARIES}
	  )
#QT5_USE_MODULES(CSPavio Widgets Core Gui OpenGL)
set_target_properties(CSPavio PROPERTIES 
			     SOVERSION ${THIS_LIB_VERSION}
			     VERSION ${THIS_LIB_VERSION}
			     )

generate_export_header(CSPavio
       BASE_NAME CSPavio
        EXPORT_MACRO_NAME CSPavio_EXPORT
        EXPORT_FILE_NAME CSPavio_Export.h
        STATIC_DEFINE CSPavio_BUILT_AS_STATIC
)
	set_std(CSPavio)

else()
add_library(CSPavio SHARED
	  ${s_qt_avio_srcs}
	  ${s_qt_avio_headers_MOC}
	  )
	set_std(CSPavio)

target_link_libraries(CSPavio PUBLIC
		   ${LIBAV_LIBRARIES}
		   Qt5::Core
		   Qt5::Gui
		   Qt5::Widgets
		   ${ADDITIONAL_DLL_LIBRARIES}
	  )

set_target_properties(CSPavio PROPERTIES 
			     SOVERSION ${THIS_LIB_VERSION}
			     VERSION ${THIS_LIB_VERSION}
			     )
INSTALL(TARGETS CSPavio DESTINATION ${LIBCSP_INSTALL_DIR})
endif()

