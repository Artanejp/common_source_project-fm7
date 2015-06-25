message("* qt/tk80bs")

set(s_qt_tk80bs_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_tk80bs_headers_MOC ${s_qt_tk80bs_headers})
else()
  QT4_WRAP_CPP(s_qt_tk80bs_headers_MOC ${s_qt_tk80bs_headers})
endif()
add_library(qt_tk80bs
	  MainWindow.cpp
	  ${s_qt_tk80bs_headers_MOC}
)


