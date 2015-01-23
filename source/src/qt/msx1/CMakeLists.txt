message("* qt/msx1")

set(s_qt_msx1_headers
	  menuclasses.h
)
QT4_WRAP_CPP(s_qt_msx1_headers_MOC ${s_qt_msx1_headers})

add_library(qt_msx1
	  MainWindow.cpp
	  ${s_qt_msx1_headers_MOC}
)


