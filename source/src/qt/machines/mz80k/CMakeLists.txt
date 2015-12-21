message("* qt/mz80k")

set(s_qt_mz80_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_mz80_headers_MOC ${s_qt_mz80_headers})
else()
  QT4_WRAP_CPP(s_qt_mz80_headers_MOC ${s_qt_mz80_headers})
endif()
add_library(qt_mz80k
	  MainWindow.cpp
	  ${s_qt_mz80_headers_MOC}
)


