message("* qt/familybasic")

set(s_qt_familybasic_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_familybasic_headers_MOC ${s_qt_familybasic_headers})
else()
  QT4_WRAP_CPP(s_qt_familybasic_headers_MOC ${s_qt_familybasic_headers})
endif()

add_library(qt_familybasic
	  MainWindow.cpp
	  ${s_qt_familybasic_headers_MOC}
)


