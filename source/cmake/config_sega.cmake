set(BUILD_SC3000 ON CACHE BOOL "Build for Sega SC-3000")
set(BUILD_GAMEGEAR ON CACHE BOOL "Build for Sega GameGear")
set(BUILD_MASTERSYSTEM ON CACHE BOOL "Build for Sega Master System")
#set(BUILD_MARK3 OFF CACHE BOOL "Build for Sega MarkIII")

if(BUILD_SC3000)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/sc3000.qrc)
	ADD_VM(sc3000 emusc3000 _SC3000)
endif()
if(BUILD_GAMEGEAR)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/gamegear.qrc)
	ADD_VM(gamegear emugamegear _GAMEGEAR)
endif()
if(BUILD_MASTERSYSTEM)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mastersystem.qrc)
	ADD_VM(gamegear emumastersystem _MASTERSYSTEM)
endif()
if(BUILD_MARK3)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mark3.qrc)
	ADD_VM(gamegear emumark3 _MARK3)
endif()