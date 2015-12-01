message("* qt/fmr30")

set(s_qt_fmr30_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_fmr30_headers_MOC ${s_qt_fmr30_headers})
else()
  QT4_WRAP_CPP(s_qt_fmr30_headers_MOC ${s_qt_fmr30_headers})
endif()

add_library(qt_fmr30
	  MainWindow.cpp
	  ${s_qt_fmr30_headers_MOC}
)


