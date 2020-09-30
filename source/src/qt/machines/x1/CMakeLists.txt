message("* qt/${EXE_NAME}")

set(s_qt_x1turboz_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_${EXE_NAME}_headers_MOC ${s_qt_x1turboz_headers})
else()
  QT4_WRAP_CPP(s_qt_${EXE_NAME}_headers_MOC ${s_qt_x1turboz_headers})
endif()
add_library(qt_${EXE_NAME}
	  MainWindow.cpp
	  ${s_qt_${EXE_NAME}_headers_MOC}
)


