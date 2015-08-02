message("* qt/ys6464a")

set(s_qt_ys6464a_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_ys6464a_headers_MOC ${s_qt_ys6464a_headers})
#  QT5_ADD_RESOURCES(s_qt_fm7_headers_RCC ${RESOURCE})
else()
  QT4_WRAP_CPP(s_qt_ys6464a_headers_MOC ${s_qt_ys6464a_headers})
endif()

add_library(qt_ys6464a
	  MainWindow.cpp
	  ${s_qt_ys6464a_headers_MOC}
)


