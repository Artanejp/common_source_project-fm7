message("* qt/pyuta")

set(s_qt_pyuta_headers
	  menuclasses.h
)

QT5_WRAP_CPP(s_qt_emupyuta_headers_MOC ${s_qt_pyuta_headers})

add_library(qt_emupyuta
	  MainWindow.cpp
	  ${s_qt_emupyuta_headers_MOC}
)


