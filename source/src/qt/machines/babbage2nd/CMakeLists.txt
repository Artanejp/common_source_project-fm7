message("* qt/babbage2nd")

set(s_qt_b2nd_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_b2nd_headers_MOC ${s_qt_b2nd_headers})
#  QT5_ADD_RESOURCES(s_qt_fm7_headers_RCC ${RESOURCE})
else()
  QT4_WRAP_CPP(s_qt_b2nd_headers_MOC ${s_qt_b2nd_headers})
endif()

add_library(qt_babbage2nd
	  MainWindow.cpp
	  ${s_qt_b2nd_headers_MOC}
)


