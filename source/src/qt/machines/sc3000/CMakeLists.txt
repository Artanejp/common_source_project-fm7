message("* qt/sc3000")

set(s_qt_sc3000_headers
	  menuclasses.h
)


QT5_WRAP_CPP(s_qt_emusc3000_headers_MOC ${s_qt_sc3000_headers})

add_library(qt_emusc3000
	  MainWindow.cpp
	  ${s_qt_emusc3000_headers_MOC}
)


