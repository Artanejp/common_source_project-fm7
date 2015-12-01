message("* qt/msx1")

set(s_qt_msx1_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_msx1_headers_MOC ${s_qt_msx1_headers})
else()
  QT4_WRAP_CPP(s_qt_msx1_headers_MOC ${s_qt_msx1_headers})
endif()

add_library(qt_msx1
	  MainWindow.cpp
	  ${s_qt_msx1_headers_MOC}
)


