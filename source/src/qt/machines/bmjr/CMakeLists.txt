message("* qt/bmjr")

set(s_qt_bmjr_headers
	  menuclasses.h
)

QT5_WRAP_CPP(s_qt_bmjr_headers_MOC ${s_qt_bmjr_headers})

add_library(qt_emubmjr
	MainWindow.cpp
	${s_qt_bmjr_headers_MOC}
)


