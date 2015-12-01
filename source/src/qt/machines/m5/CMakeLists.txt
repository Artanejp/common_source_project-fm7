message("* qt/m5")

set(s_qt_m5_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_m5_headers_MOC ${s_qt_m5_headers})
else()
  QT4_WRAP_CPP(s_qt_m5_headers_MOC ${s_qt_m5_headers})
endif()

add_library(qt_m5
	  MainWindow.cpp
	  ${s_qt_m5_headers_MOC}
)


