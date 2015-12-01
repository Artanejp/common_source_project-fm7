message("* qt/phc25")

set(s_qt_phc25_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_phc25_headers_MOC ${s_qt_phc25_headers})
else()
  QT4_WRAP_CPP(s_qt_phc25_headers_MOC ${s_qt_phc25_headers})
endif()

add_library(qt_phc25
	  MainWindow.cpp
	  ${s_qt_phc25_headers_MOC}
)


