message("* qt/fmtowns")

set(s_qt_fmtowns_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_fmtowns_headers_MOC ${s_qt_fmtowns_headers})
#  QT5_ADD_RESOURCES(s_qt_fmtowns_headers_RCC ${RESOURCE})
else()
  QT4_WRAP_CPP(s_qt_fmtowns_headers_MOC ${s_qt_fmtowns_headers})
endif()

add_library(qt_fmtowns
	  MainWindow.cpp
	  ${s_qt_fmtowns_headers_MOC}
)


