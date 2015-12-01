message("* qt/n5200")

set(s_qt_n5200_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_n5200_headers_MOC ${s_qt_n5200_headers})
else()
  QT4_WRAP_CPP(s_qt_n5200_headers_MOC ${s_qt_n5200_headers})
endif()
add_library(qt_n5200
	  MainWindow.cpp
	  ${s_qt_n5200_headers_MOC}
)


