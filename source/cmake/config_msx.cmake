set(BUILD_MSX1 ON CACHE BOOL "Build for standard MSX1")
set(BUILD_MSX2 ON CACHE BOOL "Build for standard MSX2")
set(BUILD_MSX2PLUS ON CACHE BOOL "Build for standard MSX2+")
set(BUILD_FSA1 ON CACHE BOOL "Build for Matsushita FS-A1")
set(BUILD_HX20 ON CACHE BOOL "Build for Toshiba HX20")
#set(BUILD_HBF1XDJ ON CACHE BOOL "Build for Sony HBF1XDJ")
set(BUILD_PX7 ON CACHE BOOL "Build for Pioneer PX7")
set(BUILD_SVI3X8 ON CACHE BOOL "Build for SPECTRAVIDEO SVI-3x8")

if(BUILD_MSX1)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/msx1.qrc)
	ADD_VM(msx emumsx1 _MSX1)
endif()
if(BUILD_MSX2)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/msx2.qrc)
	ADD_VM(msx emumsx2 _MSX2)
	target_compile_definitions(emumsx2 PRIVATE -D_MSX_VDP_MESS)
endif()
if(BUILD_MSX2PLUS)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/msx2plus.qrc)
	ADD_VM(msx emumsx2p _MSX2P)
	target_compile_definitions(emumsx2p PRIVATE -D_MSX_VDP_MESS)
endif()

if(BUILD_PX7)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/px7.qrc)
	ADD_VM(msx emupx7 _PX7)
endif()
if(BUILD_HX20)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/hx20.qrc)
	ADD_VM(msx emuhx20 _HX20)
	target_compile_definitions(emuhx20 PRIVATE -D_MSX_VDP_MESS)
endif()
if(BUILD_FSA1)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fsa1.qrc)
	ADD_VM(msx emufsa1 _FSA1)
	target_compile_definitions(emufsa1 PRIVATE -D_MSX_VDP_MESS)
endif()
#if(BUILD_HBF1XDJ)
#	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/hbf1xdj.qrc)
#	ADD_VM(msx emuhbf1xdj _HBF1XDJ)
#	target_compile_definitions(emuhbf1xdj PRIVATE -D_MSX_VDP_MESS)
#endif()
if(BUILD_SVI3X8)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/svi3x8.qrc)
	ADD_VM(svi3x8 emusvi3x8 _SVI3X8)
	target_compile_definitions(emufsa1 PRIVATE -D_MSX_VDP_MESS)
endif()
