message("* qt/mz3500")

set(s_qt_mz3500_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_mz3500_headers_MOC ${s_qt_mz3500_headers})
else()
  QT4_WRAP_CPP(s_qt_mz3500_headers_MOC ${s_qt_mz3500_headers})
endif()
add_library(qt_mz3500
	  MainWindow.cpp
	  ${s_qt_mz3500_headers_MOC}
)


