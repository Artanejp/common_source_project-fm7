message("* qt/pc6001")

set(s_qt_pc6001_headers
	  menuclasses.h
)
QT4_WRAP_CPP(s_qt_pc6001_headers_MOC ${s_qt_pc6001_headers})

add_library(qt_pc6001
	  MainWindow.cpp
	  ${s_qt_pc6001_headers_MOC}
)


