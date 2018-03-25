message("* qt/fm16beta")

set(s_qt_fm16beta_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_fm16beta_headers_MOC ${s_qt_fm16beta_headers})
else()
  QT4_WRAP_CPP(s_qt_fm16beta_headers_MOC ${s_qt_fm16beta_headers})
endif()

add_library(qt_fm16beta
	  MainWindow.cpp
	  ${s_qt_fm16beta_headers_MOC}
)


