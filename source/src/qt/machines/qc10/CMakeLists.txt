message("* qt/${EXE_NAME}")

set(s_qt_qc10_headers
	  menuclasses.h
)

MAKE_WRAP_CPP(s_qt_${EXE_NAME}_headers_MOC ${s_qt_qc10_headers})
add_library(qt_${EXE_NAME}
	  MainWindow.cpp
	  ${s_qt_${EXE_NAME}_headers_MOC}
)
