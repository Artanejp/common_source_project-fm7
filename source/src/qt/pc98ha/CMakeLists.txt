message("* qt/pc98ha")

set(s_qt_pc98ha_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_pc98ha_headers_MOC ${s_qt_pc98ha_headers})
else()
  QT4_WRAP_CPP(s_qt_pc98ha_headers_MOC ${s_qt_pc98ha_headers})
endif()

add_library(qt_pc98ha
	  MainWindow.cpp
	  ${s_qt_pc98ha_headers_MOC}
)


