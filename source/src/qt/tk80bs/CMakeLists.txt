message("* qt/tk80bs")

set(s_qt_tk80bs_headers
	  menuclasses.h
)
QT4_WRAP_CPP(s_qt_tk80bs_headers_MOC ${s_qt_tk80bs_headers})

add_library(qt_tk80bs
	  MainWindow.cpp
	  ${s_qt_tk80bs_headers_MOC}
)


