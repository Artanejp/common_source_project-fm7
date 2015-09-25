message("* qt/bmjr")

set(s_qt_bmjr_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_bmjr_headers_MOC ${s_qt_bmjr_headers})
else()
  QT4_WRAP_CPP(s_qt_bmjr_headers_MOC ${s_qt_bmjr_headers})
endif()

add_library(qt_bmjr
	  MainWindow.cpp
	  ${s_qt_bmjr_headers_MOC}
)


