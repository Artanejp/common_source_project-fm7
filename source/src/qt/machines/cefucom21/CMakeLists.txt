message("* qt/cefucom21")

set(s_qt_cefucom21_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_cefucom21_headers_MOC ${s_qt_cefucom21_headers})
else()
  QT4_WRAP_CPP(s_qt_cefucom21_headers_MOC ${s_qt_cefucom21_headers})
endif()
add_library(qt_cefucom21
	  MainWindow.cpp
	  ${s_qt_cefucom21_headers_MOC}
)


