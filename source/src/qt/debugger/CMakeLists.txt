message("* qt/debugger")

set(s_qt_debugger_headers
	  qt_debugger.h
)
QT4_WRAP_CPP(s_qt_debugger_headers_MOC ${s_qt_debugger_headers})

 
add_library(qt_debugger
          qt_debugger.cpp
	  ${s_qt_debugger_headers_MOC}
)

