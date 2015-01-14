message("* qt/gui")
set(s_qt_gui_headers
	  commonclasses.h
	  mainwidget.h
)
QT4_WRAP_CPP(s_qt_gui_headers_MOC ${s_qt_gui_headers})

 
add_library(qt_gui
	  menu_control.cpp
	  menu_disk.cpp
	  menu_cmt.cpp
	  menu_main.cpp
	  menu_sound.cpp
	  display_statusbar.cpp
	  ${s_qt_gui_headers_MOC}
)

