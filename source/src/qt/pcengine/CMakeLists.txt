message("* qt/pcengine")

set(s_qt_pce_headers
	  menuclasses.h
)
QT4_WRAP_CPP(s_qt_pce_headers_MOC ${s_qt_pce_headers})

add_library(qt_pce
	  MainWindow.cpp
	  ${s_qt_pce_headers_MOC}
)


