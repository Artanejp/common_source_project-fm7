message("* qt/babbage2nd")

set(s_qt_b2nd_headers
	  menuclasses.h
)

MAKE_WRAP_CPP(s_qt_b2nd_headers_MOC ${s_qt_b2nd_headers})

add_library(qt_emubabbage2nd
	  MainWindow.cpp
	  ${s_qt_b2nd_headers_MOC}
)


