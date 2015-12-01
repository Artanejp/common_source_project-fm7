message("* qt/fp1100")

set(s_qt_fp1100_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_fp1100_headers_MOC ${s_qt_fp1100_headers})
else()
  QT4_WRAP_CPP(s_qt_fp1100_headers_MOC ${s_qt_fp1100_headers})
endif()

add_library(qt_fp1100
	  MainWindow.cpp
	  ${s_qt_fp1100_headers_MOC}
)


