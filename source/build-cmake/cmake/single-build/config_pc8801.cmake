set(BUILD_PC8001 ON CACHE BOOL "Build for NEC PC-8001")
set(BUILD_PC8001MK2 ON CACHE BOOL "Build for NEC PC-8001mk2")
set(BUILD_PC8001SR ON CACHE BOOL "Build for NEC PC-8001 SR")
set(BUILD_PC8801 ON CACHE BOOL "Build for NEC PC-8801")
set(BUILD_PC8801MK2 ON CACHE BOOL "Build for NEC PC-8801mk2")
set(BUILD_PC8801MA ON CACHE BOOL "Build for NEC PC-8801 MA")

if(BUILD_PC8001)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/pc8001.qrc)
	ADD_VM(pc8801 emupc8001 _PC8001)
endif()
if(BUILD_PC8001MK2)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/pc8001mk2.qrc)
	ADD_VM(pc8801 emupc8001mk2 _PC8001MK2)
endif()
if(BUILD_PC8001SR)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/pc8001mk2sr.qrc)
	ADD_VM(pc8801 emupc8001mk2sr _PC8001SR)
endif()

if(BUILD_PC8801)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/pc8801.qrc)
	ADD_VM(pc8801 emupc8801 _PC8801)
endif()
if(BUILD_PC8801MK2)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/pc8801mk2.qrc)
	ADD_VM(pc8801 emupc8801mk2 _PC8801MK2)
endif()
if(BUILD_PC8801MA)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/pc8801ma.qrc)
	ADD_VM(pc8801 emupc8801ma _PC8801MA)
endif()
