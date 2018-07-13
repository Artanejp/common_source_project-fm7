message("* qt/bubcom80")

set(s_qt_bubcom80_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_bubcom80_headers_MOC ${s_qt_bubcom80_headers})
#  QT5_ADD_RESOURCES(s_qt_bubcom80_headers_RCC ${RESOURCE})
else()
  QT4_WRAP_CPP(s_qt_bubcom80_headers_MOC ${s_qt_bubcom80_headers})
endif()

add_library(qt_bubcom80
	  MainWindow.cpp
	  ${s_qt_bubcom80_headers_MOC}
)


