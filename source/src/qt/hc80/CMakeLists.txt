message("* qt/hc80")

set(s_qt_hc80_headers
	  menuclasses.h
)
QT4_WRAP_CPP(s_qt_hc80_headers_MOC ${s_qt_hc80_headers})

add_library(qt_hc80
	  MainWindow.cpp
	  ${s_qt_hc80_headers_MOC}
)


