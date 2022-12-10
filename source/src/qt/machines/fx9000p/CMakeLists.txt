message("* qt/fx9000p")

set(s_qt_fx9000p_headers
	  menuclasses.h
	  )

MAKE_WRAP_CPP(s_qt_emufx9000p_headers_MOC ${s_qt_fx9000p_headers})

add_library(qt_emufx9000p
	  MainWindow.cpp
	  ${s_qt_emufx9000p_headers_MOC}
)


