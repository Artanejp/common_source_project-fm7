message("* qt/x1turboz")

set(s_qt_x1turboz_headers
	  menuclasses.h
)
QT4_WRAP_CPP(s_qt_x1turboz_headers_MOC ${s_qt_x1turboz_headers})

add_library(qt_x1turboz
	  MainWindow.cpp
	  ${s_qt_x1turboz_headers_MOC}
)


