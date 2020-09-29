message("* qt/fm7")

set(s_qt_fm7_headers
	  menuclasses.h
)

 QT5_WRAP_CPP(s_qt_fm7_headers_${EXE_NAME}_MOC ${s_qt_fm7_headers})

add_library(qt_${EXE_NAME}
	  MainWindow.cpp
	  ${s_qt_fm7_headers_${EXE_NAME}_MOC}
)


