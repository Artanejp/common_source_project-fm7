message("* qt/j3100")

set(s_qt_j3100_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_j3100_headers_MOC ${s_qt_j3100_headers})
else()
  QT4_WRAP_CPP(s_qt_j3100_headers_MOC ${s_qt_j3100_headers})
endif()

add_library(qt_j3100
	  MainWindow.cpp
	  ${s_qt_j3100_headers_MOC}
)


