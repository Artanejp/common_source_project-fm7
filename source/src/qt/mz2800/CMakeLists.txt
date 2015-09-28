message("* qt/mz2800")

set(s_qt_mz2800_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_mz2800_headers_MOC ${s_qt_mz2800_headers})
else()
  QT4_WRAP_CPP(s_qt_mz2800_headers_MOC ${s_qt_mz2800_headers})
endif()
add_library(qt_mz2800
	  MainWindow.cpp
	  ${s_qt_mz2800_headers_MOC}
)


