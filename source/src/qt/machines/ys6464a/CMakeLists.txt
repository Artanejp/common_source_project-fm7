message("* qt/ys6464a")

set(s_qt_ys6464a_headers
	  menuclasses.h
)

QT5_WRAP_CPP(s_qt_ys6464a_headers_MOC ${s_qt_ys6464a_headers})

add_library(qt_emuys6464a
	  MainWindow.cpp
	  ${s_qt_ys6464a_headers_MOC}
)


