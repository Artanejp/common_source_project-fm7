message("* qt/jr800")

set(s_qt_jr800_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_jr800_headers_MOC ${s_qt_jr800_headers})
else()
  QT4_WRAP_CPP(s_qt_jr800_headers_MOC ${s_qt_jr800_headers})
endif()

add_library(qt_jr800
	  MainWindow.cpp
	  ${s_qt_jr800_headers_MOC}
)


