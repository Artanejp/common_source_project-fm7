message("* qt/${EXE_NAME}")

set(s_qt_tvboy_headers_${EXE_NAME}
	  menuclasses.h
)

MAKE_WRAP_CPP(s_qt_tvboy_headers_${EXE_NAME}_MOC ${s_qt_tvboy_headers_${EXE_NAME}})

add_library(qt_${EXE_NAME}
	  MainWindow.cpp
	  ${s_qt_tvboy_headers_${EXE_NAME}_MOC}
)


