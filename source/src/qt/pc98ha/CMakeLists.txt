message("* qt/pc98ha")

set(s_qt_pc98ha_headers
	  menuclasses.h
)
QT4_WRAP_CPP(s_qt_pc98ha_headers_MOC ${s_qt_pc98ha_headers})

add_library(qt_pc98ha
	  MainWindow.cpp
	  ${s_qt_pc98ha_headers_MOC}
)


