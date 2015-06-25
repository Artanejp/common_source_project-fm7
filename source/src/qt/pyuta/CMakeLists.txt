message("* qt/pyuta")

set(s_qt_pyuta_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_pyuta_headers_MOC ${s_qt_pyuta_headers})
else()
  QT4_WRAP_CPP(s_qt_pyuta_headers_MOC ${s_qt_pyuta_headers})
endif()
add_library(qt_pyuta
	  MainWindow.cpp
	  ${s_qt_pyuta_headers_MOC}
)


