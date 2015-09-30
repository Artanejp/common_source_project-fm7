message("* qt/rx78")

set(s_qt_rx78_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_rx78_headers_MOC ${s_qt_rx78_headers})
else()
  QT4_WRAP_CPP(s_qt_rx78_headers_MOC ${s_qt_rx78_headers})
endif()

add_library(qt_rx78
	  MainWindow.cpp
	  ${s_qt_rx78_headers_MOC}
)


