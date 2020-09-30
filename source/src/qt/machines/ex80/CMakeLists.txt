message("* qt/ex80")

set(s_qt_ex80_headers
	  menuclasses.h
)

QT5_WRAP_CPP(s_qt_ex80_headers_MOC ${s_qt_ex80_headers})

add_library(qt_emuex80
	  MainWindow.cpp
	  ${s_qt_ex80_headers_MOC}
)


