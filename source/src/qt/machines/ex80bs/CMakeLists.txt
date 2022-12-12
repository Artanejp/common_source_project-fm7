message("* qt/ex80bs")

set(s_qt_ex80bs_headers
	  menuclasses.h
)

MAKE_WRAP_CPP(s_qt_ex80bs_headers_MOC ${s_qt_ex80bs_headers})

add_library(qt_emuex80bs
	  MainWindow.cpp
	  ${s_qt_ex80bs_headers_MOC}
)


