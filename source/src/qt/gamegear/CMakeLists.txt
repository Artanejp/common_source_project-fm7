message("* qt/gamegear")

set(s_qt_sms_headers
	  menuclasses.h
)
QT4_WRAP_CPP(s_qt_sms_headers_MOC ${s_qt_sms_headers})

add_library(qt_sms
	  MainWindow.cpp
	  ${s_qt_sms_headers_MOC}
)


