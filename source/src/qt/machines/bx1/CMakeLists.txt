message("* qt/emux07")

set(s_qt_${EXE_NAME}_headers
	  menuclasses.h
)

QT5_WRAP_CPP(s_qt_${EXE_NAME}_headers_MOC ${s_qt_${EXE_NAME}_headers})
  
add_library(qt_${EXE_NAME}
	  MainWindow.cpp
	  ${s_qt_${EXE_NAME}_headers_MOC}
)


