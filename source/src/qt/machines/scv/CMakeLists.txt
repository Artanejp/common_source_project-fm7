message("* qt/scv")

set(s_qt_scv_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_scv_headers_MOC ${s_qt_scv_headers})
else()
  QT4_WRAP_CPP(s_qt_scv_headers_MOC ${s_qt_scv_headers})
endif()
add_library(qt_scv
	  MainWindow.cpp
	  ${s_qt_scv_headers_MOC}
)


