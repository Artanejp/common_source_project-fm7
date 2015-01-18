message("* qt/mz80k")

set(s_qt_mz80_headers
	  menuclasses.h
)
QT4_WRAP_CPP(s_qt_mz80_headers_MOC ${s_qt_mz80_headers})

add_library(qt_mz80
	  MainWindow.cpp
	  ${s_qt_mz80_headers_MOC}
)


