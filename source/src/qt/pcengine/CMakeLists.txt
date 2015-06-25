message("* qt/pcengine")

set(s_qt_pce_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_pce_headers_MOC ${s_qt_pce_headers})
else()
  QT4_WRAP_CPP(s_qt_pce_headers_MOC ${s_qt_pce_headers})
endif()
add_library(qt_pce
	  MainWindow.cpp
	  ${s_qt_pce_headers_MOC}
)


