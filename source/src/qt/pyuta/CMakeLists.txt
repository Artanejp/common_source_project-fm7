message("* qt/pyuta")

set(s_qt_pyuta_headers
	  menuclasses.h
)
QT4_WRAP_CPP(s_qt_pyuta_headers_MOC ${s_qt_pyuta_headers})

add_library(qt_pyuta
	  MainWindow.cpp
	  ${s_qt_pyuta_headers_MOC}
)


