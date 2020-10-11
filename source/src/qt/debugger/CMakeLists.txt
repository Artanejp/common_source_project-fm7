message("* qt/debugger_${EXE_NAME}")

set(s_qt_debugger_headers
	  qt_debugger.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_debugger_headers_MOC ${s_qt_debugger_headers})
else()
  QT4_WRAP_CPP(s_qt_debugger_headers_MOC ${s_qt_debugger_headers})
endif()
 
add_library(qt_debugger_${EXE_NAME}
		  qt_debugger.cpp
		  ${s_qt_debugger_headers_MOC}
)
set_std(qt_debugger_${EXE_NAME})

