message("* qt/pv1000")

set(s_qt_pv1000_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_pv1000_headers_MOC ${s_qt_pv1000_headers})
else()
  QT4_WRAP_CPP(s_qt_pv1000_headers_MOC ${s_qt_pv1000_headers})
endif()

add_library(qt_pv1000
	  MainWindow.cpp
	  ${s_qt_pv1000_headers_MOC}
)


