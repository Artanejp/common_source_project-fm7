message("* qt/emux07")

set(s_qt_x07_headers
	  menuclasses.h
)

MAKE_WRAP_CPP(s_qt_emux07_headers_MOC ${s_qt_x07_headers})
  
add_library(qt_emux07
	  MainWindow.cpp
	  ${s_qt_emux07_headers_MOC}
)


