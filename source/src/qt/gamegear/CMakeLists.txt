message("* qt/gamegear")

set(s_qt_sms_headers
	  menuclasses.h
	  )

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_sms_headers_MOC ${s_qt_sms_headers})
else()
  QT4_WRAP_CPP(s_qt_sms_headers_MOC ${s_qt_sms_headers})
endif()
  
add_library(qt_sms
	  MainWindow.cpp
	  ${s_qt_sms_headers_MOC}
)


