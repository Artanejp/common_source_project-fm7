message("* qt/emuyis")

set(s_qt_yis_headers
	  menuclasses.h
)

MAKE_WRAP_CPP(s_qt_emuyis_headers_MOC ${s_qt_yis_headers})

add_library(qt_emuyis
	  MainWindow.cpp
	  ${s_qt_emuyis_headers_MOC}
)


