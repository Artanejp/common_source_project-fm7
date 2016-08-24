message("* qt/gui")

set(s_qt_gui_headers
	  agar_logger.h
  
	  qt_dialogs.h
	  qt_gldraw.h
	  qt_glutil_gl2_0.h
	  qt_glutil_gl3_0.h
	  
	  commonclasses.h
	  sound_dialog.h
	  mainwidget_base.h
	  menu_metaclass.h
	  menu_disk.h
	  menu_cmt.h
	  menu_cart.h
	  menu_quickdisk.h
	  menu_compactdisc.h
	  menu_laserdisc.h
	  menu_binary.h
	  menu_bubble.h
	  display_about.h
	  display_text_document.h
	  dropdown_keyset.h
	  dropdown_joystick.h
	  dropdown_jsbutton.h
	  dropdown_jspage.h
	  dialog_set_key.h
	  dialog_set_key_combo.h
	  dialog_movie.h

	  tab_movie_general.h
	  tab_movie_h264.h
	  tab_movie_mpeg4.h
	  
	  draw_thread.h
	  joy_thread.h
)

set(s_qt_gui_srcs
	  emuevents_control.cpp
	  qt_gldraw.cpp
	  qt_glutil.cpp
	  qt_glutil_gl2_0.cpp
	  qt_glutil_gl3_0.cpp
	  qt_glevents.cpp
	  qt_dialogs.cpp

          qt_input.cpp
	  menu_control.cpp
	  menu_metaclass.cpp
	  menu_disk.cpp
	  menu_quickdisk.cpp
	  menu_compactdisc.cpp
	  menu_laserdisc.cpp
	  menu_cmt.cpp
	  menu_cart.cpp
	  menu_main.cpp
	  menu_sound.cpp
	  menu_binary.cpp
	  menu_bubble.cpp
	  menu_screen.cpp
	  mainwindow_utils.cpp
	  util_fd.cpp
	  util_cd.cpp
	  util_ld.cpp
	  util_cmt.cpp
	  util_qd.cpp
	  util_cart.cpp
	  util_binary.cpp
	  util_bubble.cpp
	  display_statusbar.cpp
	  display_about.cpp
	  display_text_document.cpp
	  sound_dialog.cpp
	  dropdown_keyset.cpp
	  dropdown_joystick.cpp
	  dropdown_jsbutton.cpp
	  dropdown_jspage.cpp
	  dialog_set_key.cpp
	  dialog_set_key_combo.cpp
	  dialog_movie.cpp
	  
	  tab_movie_general.cpp
	  tab_movie_h264.cpp
	  tab_movie_mpeg4.cpp
	  
	  draw_thread.cpp
	  joy_thread.cpp
	  agar_logger.cpp
	  
	  qt_main.cpp
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_gui_headers_MOC ${s_qt_gui_headers})
  QT5_ADD_RESOURCES(s_qt_gui_RCC ${RESOURCE}
                    ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/commontexts.qrc
                    ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/shaders.qrc
		    )
else()
  QT4_WRAP_CPP(s_qt_gui_headers_MOC ${s_qt_gui_headers})
endif()

if(WIN32)
add_library(qt_gui
	  ${s_qt_gui_srcs}
	  ${s_qt_gui_headers_MOC}
	  ${s_qt_gui_RCC}
)
#include (GenerateExportHeader)
#add_library(CSPgui SHARED
#	  ${s_qt_gui_srcs}
##	  ${s_qt_gui_headers_MOC}
#	  )
#target_link_libraries(CSPgui PUBLIC
#	   ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPemu_utils/build-win32/qt/emuutils/libemu_utils.a
#	   ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPavio/build-win32/qt/avio/libqt_avio.a
#	   ${CMAKE_SOURCE_DIR}/../../build-cmake/libCSPosd/build-win32/qt/osd/libqt_osd.a
#	  Qt5::Core
#	  Qt5::Gui
#	  Qt5::OpenGL
#	  Qt5::Widgets
#	  ${SDL2_LIBRARIES}
#	  )
#QT5_USE_MODULES(CSPgui Widgets Core Gui OpenGL)
#generate_export_header(CSPgui
#       BASE_NAME CSPgui
#        EXPORT_MACRO_NAME CSPgui_EXPORT
#        EXPORT_FILE_NAME CSPgui_Export.h
#        STATIC_DEFINE CSPgui_BUILT_AS_STATIC
#)

else()
add_library(CSPgui SHARED
	  ${s_qt_gui_srcs}
	  ${s_qt_gui_headers_MOC}
	  ${s_qt_gui_RCC}
)
target_link_libraries(CSPgui PUBLIC
	  Qt5::Core
	  Qt5::Gui
	  Qt5::OpenGL
	  Qt5::Widgets
	  ${SDL2_LIBRARIES}
	  )

set_target_properties(CSPgui PROPERTIES 
			     SOVERSION 2.1.0
			     VERSION 2.1.0
			     )
INSTALL(TARGETS CSPgui DESTINATION ${LIBCSP_INSTALL_DIR})
endif()

