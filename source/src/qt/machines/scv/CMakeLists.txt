message("* qt/scv")

set(s_qt_scv_headers
	  menuclasses.h
)

MAKE_WRAP_CPP(s_qt_emuscv_headers_MOC ${s_qt_scv_headers})

add_library(qt_emuscv
	  MainWindow.cpp
	  ${s_qt_emuscv_headers_MOC}
)


