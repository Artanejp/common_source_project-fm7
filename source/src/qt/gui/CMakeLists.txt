message("* qt/gui")
set(s_qt_gui_headers
	  commonclasses.h
	  sound_dialog.h
	  mainwidget.h
	  menu_metaclass.h
	  menu_disk.h
	  menu_cmt.h
	  menu_cart.h
	  menu_quickdisk.h
	  menu_binary.h
	  display_about.h
	  display_text_document.h
	  dropdown_keyset.h
	  dropdown_joystick.h
	  dropdown_jspage.h
	  dropdown_jsbutton.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_gui_headers_MOC ${s_qt_gui_headers})
else()
  QT4_WRAP_CPP(s_qt_gui_headers_MOC ${s_qt_gui_headers})
endif()
 
add_library(qt_gui
          qt_input.cpp
	  menu_control.cpp
	  menu_metaclass.cpp
	  menu_disk.cpp
	  menu_quickdisk.cpp
	  menu_cmt.cpp
	  menu_cart.cpp
	  menu_main.cpp
	  menu_sound.cpp
	  menu_binary.cpp
	  menu_screen.cpp
	  mainwindow_utils.cpp
	  util_fd.cpp
	  util_cmt.cpp
	  util_qd.cpp
	  util_cart.cpp
	  util_binary.cpp
	  display_statusbar.cpp
	  display_about.cpp
	  display_text_document.cpp
	  sound_dialog.cpp
	  dropdown_keyset.cpp
	  dropdown_joystick.cpp
	  dropdown_jspage.cpp
	  dropdown_jsbutton.cpp
	  ${s_qt_gui_headers_MOC}
)

