message("* qt/pv2000")

set(s_qt_pv2000_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_pv2000_headers_MOC ${s_qt_pv2000_headers})
else()
  QT4_WRAP_CPP(s_qt_pv2000_headers_MOC ${s_qt_pv2000_headers})
endif()

add_library(qt_pv2000
	  MainWindow.cpp
	  ${s_qt_pv2000_headers_MOC}
)


