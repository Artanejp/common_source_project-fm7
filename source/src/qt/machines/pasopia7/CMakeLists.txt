message("* qt/pasopia7")

set(s_qt_pasopia7_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_pasopia7_headers_MOC ${s_qt_pasopia7_headers})
else()
  QT4_WRAP_CPP(s_qt_pasopia7_headers_MOC ${s_qt_pasopia7_headers})
endif()

add_library(qt_pasopia7
	  MainWindow.cpp
	  ${s_qt_pasopia7_headers_MOC}
)


