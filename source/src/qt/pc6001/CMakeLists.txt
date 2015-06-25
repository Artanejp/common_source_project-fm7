message("* qt/pc6001")

set(s_qt_pc6001_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_pc6001_headers_MOC ${s_qt_pc6001_headers})
else()
  QT4_WRAP_CPP(s_qt_pc6001_headers_MOC ${s_qt_pc6001_headers})
endif()
add_library(qt_pc6001
	  MainWindow.cpp
	  ${s_qt_pc6001_headers_MOC}
)


