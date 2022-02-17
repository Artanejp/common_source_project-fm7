message("* qt/smb80te")

set(s_qt_smb80te_headers
	  menuclasses.h
)

MAKE_WRAP_CPP(s_qt_smb80te_headers_MOC ${s_qt_smb80te_headers})

add_library(qt_emusmb80te
	  MainWindow.cpp
	  ${s_qt_smb80te_headers_MOC}
)


