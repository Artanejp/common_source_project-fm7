message("* qt/yis")

set(s_qt_yis_headers
	  menuclasses.h
)

QT5_WRAP_CPP(s_qt_yis_headers_MOC ${s_qt_yis_headers})

add_library(qt_emuyis
	  MainWindow.cpp
	  ${s_qt_yis_headers_MOC}
)


