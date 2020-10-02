message("* qt/${EXE_NAME}")

set(s_qt_pce_headers
	  menuclasses.h
)

QT5_WRAP_CPP(s_qt_${EXE_NAME}_headers_MOC ${s_qt_pce_headers})

add_library(qt_${EXE_NAME}
	  MainWindow.cpp
	  ${s_qt_${EXE_NAME}_headers_MOC}
)


