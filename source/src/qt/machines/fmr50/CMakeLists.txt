message("* qt/fmr50")

set(s_qt_fmr50_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_fmr50_headers_MOC ${s_qt_fmr50_headers})
else()
  QT4_WRAP_CPP(s_qt_fmr50_headers_MOC ${s_qt_fmr50_headers})
endif()

add_library(qt_fmr50
	  MainWindow.cpp
	  ${s_qt_fmr50_headers_MOC}
)


