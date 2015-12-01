message("* qt/hc40")

set(s_qt_hc40_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_hc40_headers_MOC ${s_qt_hc40_headers})
else()
  QT4_WRAP_CPP(s_qt_hc40_headers_MOC ${s_qt_hc40_headers})
endif()

add_library(qt_hc40
	  MainWindow.cpp
	  ${s_qt_hc40_headers_MOC}
)


