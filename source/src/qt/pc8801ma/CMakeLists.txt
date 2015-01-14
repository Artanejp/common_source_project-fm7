message("* qt/pc8801ma")

set(s_qt_pc8801ma_headers
	  menuclasses.h
)
QT4_WRAP_CPP(s_qt_pc8801ma_headers_MOC ${s_qt_pc8801ma_headers})

add_library(qt_pc8801
	  MainWindow.cpp
	  ${s_qt_pc8801ma_headers_MOC}
)


