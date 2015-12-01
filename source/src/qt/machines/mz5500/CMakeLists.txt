message("* qt/mz5500")

set(s_qt_mz5500_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_mz5500_headers_MOC ${s_qt_mz5500_headers})
else()
  QT4_WRAP_CPP(s_qt_mz5500_headers_MOC ${s_qt_mz5500_headers})
endif()
add_library(qt_mz5500
	  MainWindow.cpp
	  ${s_qt_mz5500_headers_MOC}
)


