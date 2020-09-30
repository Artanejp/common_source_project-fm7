message("* qt/x07")

set(s_qt_x07_headers
	  menuclasses.h
)

QT5_WRAP_CPP(s_qt_x07_headers_MOC ${s_qt_x07_headers})
  
add_library(qt_emux07
	  MainWindow.cpp
	  ${s_qt_x07_headers_MOC}
)


