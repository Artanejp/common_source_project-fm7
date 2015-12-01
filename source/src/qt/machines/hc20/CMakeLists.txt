message("* qt/hc20")

set(s_qt_hc20_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_hc20_headers_MOC ${s_qt_hc20_headers})
else()
  QT4_WRAP_CPP(s_qt_hc20_headers_MOC ${s_qt_hc20_headers})
endif()

add_library(qt_hc20
	  MainWindow.cpp
	  ${s_qt_hc20_headers_MOC}
)


