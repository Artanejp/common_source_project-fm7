message("* qt/pcengine")

set(s_qt_pce_headers
	  menuclasses.h
)

QT5_WRAP_CPP(s_qt_emupcengine_headers_MOC ${s_qt_pce_headers})

add_library(qt_emupcengine
	  MainWindow.cpp
	  ${s_qt_emupcengine_headers_MOC}
)


