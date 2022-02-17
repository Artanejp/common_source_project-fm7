message("* qt/yalky")

set(s_qt_yalky_headers
	  menuclasses.h
)

MAKE_WRAP_CPP(s_qt_yalky_headers_MOC ${s_qt_yalky_headers})

add_library(qt_emuyalky
	  MainWindow.cpp
	  ${s_qt_yalky_headers_MOC}
)


