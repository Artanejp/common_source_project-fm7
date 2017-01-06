message("* qt/scv")

set(s_qt_colecovision_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_colecovision_headers_MOC ${s_qt_colecovision_headers})
else()
  QT4_WRAP_CPP(s_qt_colecovision_headers_MOC ${s_qt_colecovision_headers})
endif()
add_library(qt_colecovision
	  MainWindow.cpp
	  ${s_qt_colecovision_headers_MOC}
)


