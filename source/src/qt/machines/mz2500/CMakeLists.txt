message("* qt/mz2500")

set(s_qt_mz2500_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_mz2500_headers_MOC ${s_qt_mz2500_headers})
else()
  QT4_WRAP_CPP(s_qt_mz2500_headers_MOC ${s_qt_mz2500_headers})
endif()
add_library(qt_mz2500
	  MainWindow.cpp
	  ${s_qt_mz2500_headers_MOC}
)


