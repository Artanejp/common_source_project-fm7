message("* qt/babbage2nd")

set(s_qt_trnjr_headers
	  menuclasses.h
)

MAKE_WRAP_CPP(s_qt_trnjr_headers_MOC ${s_qt_trnjr_headers})

add_library(qt_emutrnjr
	  MainWindow.cpp
	  ${s_qt_trnjr_headers_MOC}
)


