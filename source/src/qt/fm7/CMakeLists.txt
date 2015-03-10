message("* qt/fm7")

set(s_qt_fm7_headers
	  menuclasses.h
)
QT4_WRAP_CPP(s_qt_fm7_headers_MOC ${s_qt_fm7_headers})

add_library(qt_fm7
	  MainWindow.cpp
	  ${s_qt_fm7_headers_MOC}
)


