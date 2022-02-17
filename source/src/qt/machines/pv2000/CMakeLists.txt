message("* qt/emupv2000")

set(s_qt_emupv2000_headers
	  menuclasses.h
)

MAKE_WRAP_CPP(s_qt_emupv2000_headers_MOC ${s_qt_emupv2000_headers})

add_library(qt_emupv2000
	  MainWindow.cpp
	  ${s_qt_emupv2000_headers_MOC}
)


