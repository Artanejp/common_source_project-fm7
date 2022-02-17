message("* qt/emupv1000")

set(s_qt_pv1000_headers
	  menuclasses.h
)

MAKE_WRAP_CPP(s_qt_emupv1000_headers_MOC ${s_qt_pv1000_headers})
add_library(qt_emupv1000
	  MainWindow.cpp
	  ${s_qt_emupv1000_headers_MOC}
)


