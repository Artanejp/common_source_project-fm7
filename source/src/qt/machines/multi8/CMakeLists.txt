message("* qt/multi8")

set(s_qt_multi8_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_multi8_headers_MOC ${s_qt_multi8_headers})
else()
  QT4_WRAP_CPP(s_qt_multi8_headers_MOC ${s_qt_multi8_headers})
endif()

add_library(qt_multi8
	  MainWindow.cpp
	  ${s_qt_multi8_headers_MOC}
)


