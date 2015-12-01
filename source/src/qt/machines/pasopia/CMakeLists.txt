message("* qt/pasopia")

set(s_qt_pasopia_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_pasopia_headers_MOC ${s_qt_pasopia_headers})
else()
  QT4_WRAP_CPP(s_qt_pasopia_headers_MOC ${s_qt_pasopia_headers})
endif()

add_library(qt_pasopia
	  MainWindow.cpp
	  ${s_qt_pasopia_headers_MOC}
)


