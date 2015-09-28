message("* qt/qc10")

set(s_qt_qc10_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_qc10_headers_MOC ${s_qt_qc10_headers})
else()
  QT4_WRAP_CPP(s_qt_qc10_headers_MOC ${s_qt_qc10_headers})
endif()

add_library(qt_qc10
	  MainWindow.cpp
	  ${s_qt_qc10_headers_MOC}
)


