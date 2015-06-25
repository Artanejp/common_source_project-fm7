message("* qt/fp200")

set(s_qt_fp200_headers
	  menuclasses.h
	  )
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_fp200_headers_MOC ${s_qt_fp200_headers})
else()
  QT4_WRAP_CPP(s_qt_fp200_headers_MOC ${s_qt_fp200_headers})
endif()

add_library(qt_fp200
	  MainWindow.cpp
	  ${s_qt_fp200_headers_MOC}
)


