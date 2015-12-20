message("* qt/x07")

set(s_qt_x07_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_x07_headers_MOC ${s_qt_x07_headers})
else()
  QT4_WRAP_CPP(s_qt_x07_headers_MOC ${s_qt_x07_headers})
endif()
add_library(qt_x07
	  MainWindow.cpp
	  ${s_qt_x07_headers_MOC}
)


