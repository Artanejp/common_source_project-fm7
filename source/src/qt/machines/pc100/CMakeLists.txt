message("* qt/pc100")

set(s_qt_pc100_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_pc100_headers_MOC ${s_qt_pc100_headers})
else()
  QT4_WRAP_CPP(s_qt_pc100_headers_MOC ${s_qt_pc100_headers})
endif()

add_library(qt_pc100
	  MainWindow.cpp
	  ${s_qt_pc100_headers_MOC}
)


