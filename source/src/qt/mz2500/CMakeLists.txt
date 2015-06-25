message("* qt/mz2500")

set(s_qt_mz2500_headers
	  menuclasses.h
)
QT4_WRAP_CPP(s_qt_mz2500_headers_MOC ${s_qt_mz2500_headers})

add_library(qt_mz2500
	  MainWindow.cpp
	  ${s_qt_mz2500_headers_MOC}
)


