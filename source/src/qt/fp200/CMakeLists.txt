message("* qt/fp200")

set(s_qt_fp200_headers
	  menuclasses.h
)
QT4_WRAP_CPP(s_qt_fp200_headers_MOC ${s_qt_fp200_headers})

add_library(qt_fp200
	  MainWindow.cpp
	  ${s_qt_fp200_headers_MOC}
)


