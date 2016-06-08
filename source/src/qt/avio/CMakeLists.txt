message("* qt/avio")

set(s_qt_avio_headers
	  movie_saver.h
)

set(s_qt_avio_srcs
	  movie_saver.cpp
	  movie_saver_audio.cpp
	  movie_saver_video.cpp
	  movie_saver_fileio.cpp
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

#add_library(CSPavio SHARED
#	  ${s_qt_avio_srcs}
#	  ${s_qt_avio_headers_MOC}
#)

#target_link_libraries(CSPavio PUBLIC
#		   ${LIBAV_LIBRARIES}
#          ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPavio/build-win32/qt/avio/CSPavio.dll
#		   Qt5::Core
#		   Qt5::Gui
#		   Qt5::Widgets
#	  )
#QT5_USE_MODULES(CSPavio Widgets Core Gui OpenGL)
#
#generate_export_header(CSPavio
#       BASE_NAME CSPavio
#        EXPORT_MACRO_NAME CSPavio_EXPORT
#        EXPORT_FILE_NAME CSPavio_Export.h
#        STATIC_DEFINE CSPavio_BUILT_AS_STATIC
#)

else()
#add_library(CSPavio SHARED
#	  ${s_qt_avio_srcs}
#	  ${s_qt_avio_headers_MOC}
#)
#SET(CMAKE_FIND_LIBRARY_SUFFIXES .a)
#find_library(LIBAV_STATIC_LIBAVCODEC  avcodec)
#find_library(LIBAV_STATIC_LIBAVDEVICE avdevice)
#find_library(LIBAV_STATIC_LIBAVFILTER avfilter)
#find_library(LIBAV_STATIC_LIBAVFORMAT avformat)
#find_library(LIBAV_STATIC_LIBAVUTIL avutil)
#find_library(LIBAV_STATIC_LIBAVRESAMPLE avresample)
#find_library(LIBAV_STATIC_LIBSWRESAMPLE swresample)
#find_library(LIBAV_STATIC_LIBSWSCALE swscale)
#find_library(LIBAV_STATIC_LIBPOSTPROC postproc)


#if(NOT (LIBAV_STATIC_LIBAVCODEC-NOTFOUND OR LIBAV_STATIC_LIBAVDEVICE-NOTFOUND
#  OR LIBAV_STATIC_LIBAVFILTER-NOTFOUND OR LIBAV_STATIC_LIBAVFORMAT-NOTFOUND 
#   OR LIBAV_STATIC_LIBAVUTIL-NOTFOUND  OR LIBAV_STATIC_LIBAVRESAMPLE-NOTFOUND 
#   OR LIBAV_STATIC_LIBSWRESAMPLE-NOTFOUND OR LIBAV_STATIC_LIBSWSCAPE-NOTFOUND
#   OR LIBAV_STATIC_LIBPOSTPROC-NOTFOUND ))
add_library(CSPavio SHARED
	  ${s_qt_avio_srcs}
	  ${s_qt_avio_headers_MOC}
	  )
#set(LIBAV_STATIC_PREFIX /usr/local/ffmpeg-2.8.7/lib)
#target_link_libraries(CSPavio LINK_PRIVATE
#	  ${LIBAV_STATIC_PREFIX}/libavcodec.a
#	  ${LIBAV_STATIC_PREFIX}/libavfilter.a
#	  ${LIBAV_STATIC_PREFIX}/libavformat.a
#	  ${LIBAV_STATIC_PREFIX}/libavutil.a
#	  avresample
#	  ${LIBAV_STATIC_PREFIX}/libswresample.a
#	  ${LIBAV_STATIC_PREFIX}/libswscale.a
#	  ${LIBAV_STATIC_PREFIX}/libpostproc.a
#	  )
#else()
#   add_library(CSPavio SHARED
#	  ${s_qt_avio_srcs}
#	  ${s_qt_avio_headers_MOC}
#	  )
#endif()

set_target_properties(CSPavio PROPERTIES 
			     SOVERSION 1.2.0
			     VERSION 1.2.0
			     )
INSTALL(TARGETS CSPavio DESTINATION ${LIBCSP_INSTALL_DIR})
endif()

