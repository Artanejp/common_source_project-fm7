message("* qt/cefucom21")

set(s_qt_cefucom21_headers
	  menuclasses.h
)
	
MAKE_WRAP_CPP(s_qt_cefucom21_headers_MOC ${s_qt_cefucom21_headers})
add_library(qt_emucefucom21
	  MainWindow.cpp
	  ${s_qt_cefucom21_headers_MOC}
)


