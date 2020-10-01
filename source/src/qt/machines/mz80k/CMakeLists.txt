message("* qt/mz80k")

set(s_qt_mz80_headers
	  menuclasses.h
)

QT5_WRAP_CPP(s_qt_${EXE_NAME}_headers_MOC ${s_qt_mz80_headers})
add_library(qt_${EXE_NAME}
	  MainWindow.cpp
	  ${s_qt_${EXE_NAME}_headers_MOC}
)


