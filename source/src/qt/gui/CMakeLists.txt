message("* qt/gui")

set(s_qt_gui_headers
	  qt_dialogs.h
	  qt_gldraw.h
	  qt_glutil_gl2_0.h
	  qt_glutil_gl3_0.h
	  
	  commonclasses.h
	  sound_dialog.h
	  mainwidget_base.h
	  menu_disk.h
	  menu_cmt.h
	  menu_cart.h
	  menu_quickdisk.h
	  menu_compactdisc.h
	  menu_binary.h
	  menu_bubble.h
#	  display_about.h
	  display_text_document.h
	  dropdown_keyset.h
	  dropdown_joystick.h
	  dropdown_jsbutton.h
	  dialog_set_key.h
	  dialog_set_key_combo.h
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
	  menu_disk.cpp
	  menu_quickdisk.cpp
	  menu_compactdisc.cpp
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
	  util_cmt.cpp
	  util_qd.cpp
	  util_cart.cpp
	  util_binary.cpp
	  util_bubble.cpp
	  display_statusbar.cpp
#	  display_about.cpp
	  display_text_document.cpp
	  sound_dialog.cpp
	  dropdown_keyset.cpp
	  dropdown_joystick.cpp
	  dropdown_jsbutton.cpp
	  dialog_set_key.cpp
	  dialog_set_key_combo.cpp
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_gui_headers_MOC ${s_qt_gui_headers})
else()
  QT4_WRAP_CPP(s_qt_gui_headers_MOC ${s_qt_gui_headers})
endif()

if(WIN32)
add_library(qt_gui
	  ${s_qt_gui_srcs}
	  ${s_qt_gui_headers_MOC}
)

else()
add_library(CSPgui SHARED
	  ${s_qt_gui_srcs}
	  ${s_qt_gui_headers_MOC}
)

set_target_properties(CSPgui PROPERTIES 
			     SOVERSION 1.0.0
			     VERSION 1.0.0
			     )
INSTALL(TARGETS CSPgui DESTINATION ${LIBCSP_INSTALL_DIR})
endif()

