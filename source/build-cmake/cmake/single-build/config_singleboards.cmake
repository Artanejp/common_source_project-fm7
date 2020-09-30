set(BUILD_BABBAGE2ND ON CACHE BOOL "Build Gijutsu-Hyoron-Sha Babbage-2nd")

set(BUILD_CEFUCOM21 ON CACHE BOOL "Build Hino Electronics CEFUCOM-21")

set(BUILD_EX80 ON CACHE BOOL "Build TOSHIBA EX-80")

set(BUILD_SMB80TE ON CACHE BOOL "Build SHARP SM-B-80TE")

set(BUILD_TK80 ON CACHE BOOL "Build NEC TK-80")
set(BUILD_TK80BS ON CACHE BOOL "Build NEC TK-80BS")
set(BUILD_TK85 ON CACHE BOOL "Build NEC TK-85")

set(BUILD_YALKY ON CACHE BOOL "Build Yuasa Kyouiku System YALKY")
set(BUILD_YS6464A ON CACHE BOOL "Build SHINKO SANGYO YS-6464A")


if(BUILD_BABBAGE2ND)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/babbage2nd.qrc)
	ADD_VM(babbage2nd emubabbage2nd _BABBAGE2ND)
endif()
if(BUILD_CEFUCOM21)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/cefucom21.qrc)
	ADD_VM(cefucom21 emucefucom21 _CEFUCOM21)
endif()
if(BUILD_EX80)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/ex80.qrc)
	ADD_VM(ex80 emuex80 _EX80)
endif()

if(BUILD_SMB80TE)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/smb80te.qrc)
	ADD_VM(smb80te emusmb80te _SMB80TE)
endif()

if(BUILD_TK80)
	# still not implemented 20200930 K.O
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/tk80.qrc)
	ADD_VM(tk80bs emutk80 _TK80)
endif()
if(BUILD_TK80BS)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/tk80bs.qrc)
	ADD_VM(tk80bs emutk80bs _TK80BS)
endif()
if(BUILD_TK85)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/tk85.qrc)
	ADD_VM(tk80bs emutk85 _TK85)
endif()

if(BUILD_YALKY)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/yalky.qrc)
	ADD_VM(yalky emuyalky _YALKY)
endif()

if(BUILD_YS6464A)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/ys6464a.qrc)
	ADD_VM(ys6464a emuys6464a _YS6464A)
endif()
