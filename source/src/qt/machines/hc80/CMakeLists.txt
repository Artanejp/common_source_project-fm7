message("* qt/hc80")

set(s_qt_hc80_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_hc80_headers_MOC ${s_qt_hc80_headers})
else()
  QT4_WRAP_CPP(s_qt_hc80_headers_MOC ${s_qt_hc80_headers})
endif()

add_library(qt_hc80
	  MainWindow.cpp
	  ${s_qt_hc80_headers_MOC}
)


