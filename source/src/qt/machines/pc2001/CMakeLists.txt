message("* qt/pc2001")

set(s_qt_pc2001_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_pc2001_headers_MOC ${s_qt_pc2001_headers})
else()
  QT4_WRAP_CPP(s_qt_pc2001_headers_MOC ${s_qt_pc2001_headers})
endif()
add_library(qt_pc2001
	  MainWindow.cpp
	  ${s_qt_pc2001_headers_MOC}
)


