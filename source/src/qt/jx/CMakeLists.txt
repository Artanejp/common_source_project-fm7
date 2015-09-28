message("* qt/jx")

set(s_qt_jx_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_jx_headers_MOC ${s_qt_jx_headers})
else()
  QT4_WRAP_CPP(s_qt_jx_headers_MOC ${s_qt_jx_headers})
endif()

add_library(qt_jx
	  MainWindow.cpp
	  ${s_qt_jx_headers_MOC}
)


