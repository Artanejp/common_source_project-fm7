message("* qt/jr100")

set(s_qt_jr100_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_jr100_headers_MOC ${s_qt_jr100_headers})
else()
  QT4_WRAP_CPP(s_qt_jr100_headers_MOC ${s_qt_jr100_headers})
endif()

add_library(qt_jr100
	  MainWindow.cpp
	  ${s_qt_jr100_headers_MOC}
)


