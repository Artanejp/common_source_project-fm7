message("* qt/fm16pi")

set(s_qt_fm16pi_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_fm16pi_headers_MOC ${s_qt_fm16pi_headers})
else()
  QT4_WRAP_CPP(s_qt_fm16pi_headers_MOC ${s_qt_fm16pi_headers})
endif()

add_library(qt_fm16pi
	  MainWindow.cpp
	  ${s_qt_fm16pi_headers_MOC}
)


