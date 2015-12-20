message("* qt/ex80")

set(s_qt_ex80_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_ex80_headers_MOC ${s_qt_ex80_headers})
else()
  QT4_WRAP_CPP(s_qt_ex80_headers_MOC ${s_qt_ex80_headers})
endif()

add_library(qt_ex80
	  MainWindow.cpp
	  ${s_qt_ex80_headers_MOC}
)


