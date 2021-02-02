message("* qt/emux07")

set(s_qt_bx1_headers
	  menuclasses.h
)

QT5_WRAP_CPP(s_qt_emubx1_headers_MOC ${s_qt_bx1_headers})
  
add_library(qt_emubx1
	  MainWindow.cpp
	  ${s_qt_emubx1_headers_MOC}
)


