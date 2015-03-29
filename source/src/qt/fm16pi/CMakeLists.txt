message("* qt/fm16pi")

set(s_qt_fm16pi_headers
	  menuclasses.h
)
QT4_WRAP_CPP(s_qt_fm16pi_headers_MOC ${s_qt_fm16pi_headers})

add_library(qt_fm16pi
	  MainWindow.cpp
	  ${s_qt_fm16pi_headers_MOC}
)


