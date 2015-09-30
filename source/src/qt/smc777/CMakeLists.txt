message("* qt/smc777")

set(s_qt_smc777_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_smc777_headers_MOC ${s_qt_smc777_headers})
else()
  QT4_WRAP_CPP(s_qt_smc777_headers_MOC ${s_qt_smc777_headers})
endif()

add_library(qt_smc777
	  MainWindow.cpp
	  ${s_qt_smc777_headers_MOC}
)


