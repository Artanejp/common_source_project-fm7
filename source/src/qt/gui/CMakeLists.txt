message("* qt/gui")
set(s_qt_gui_headers
	  commonclasses.h
)
QT4_WRAP_CPP(s_qt_gui_headers_MOC ${s_qt_gui_headers})

 
add_library(qt_gui
	  menu_control.cpp
	  menu_disk.cpp
	  ${s_qt_gui_headers_MOC}
)

