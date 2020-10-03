# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

message("")
message("** Start of configure CommonSourceProject,FM-8/7/77/AV, Qt **")
message("")

set(USE_FMGEN ON)
set(WITH_DEBUGGER ON)
set(WITH_MOUSE ON)
set(WITH_JOYSTICK ON)

#set(VMFILES_FM7
#	   event.cpp
#)

set(VMFILES_LIB_FM7
   and.cpp
   datarec.cpp
   ym2203.cpp
   pcm1bit.cpp
   disk.cpp
   mb8877.cpp
   prnfile.cpp
   or.cpp
   noise.cpp
   i8251.cpp
)

set(BUILD_FM7 ON CACHE BOOL "Build for FM7")
set(BUILD_FMNEW7 ON CACHE BOOL "Build for FM7")
set(BUILD_FM8 ON CACHE BOOL "Build for FM8")
set(BUILD_FM77 ON CACHE BOOL "Build for FM77")
#set(BUILD_FM77L2 ON CACHE BOOL "Build for FM77L2")
set(BUILD_FM77L4 ON CACHE BOOL "Build for FM77L4")
set(BUILD_FM77AV ON CACHE BOOL "Build for FM77AV")
#set(BUILD_FM77AV20 ON CACHE BOOL "Build for FM77AV20")
set(BUILD_FM77AV40 ON CACHE BOOL "Build for FM77AV40")
set(BUILD_FM77AV40SX ON CACHE BOOL "Build for FM77AV40SX")
set(BUILD_FM77AV40EX ON CACHE BOOL "Build for FM77AV40EX")
set(FM77_EXTRAM_PAGES  "3" CACHE STRING "Set banks of EXTRAM of FM77, bank = 64Kbytes")
#set(FM77L2_EXTRAM_PAGES  "3" CACHE STRING "Set banks of EXTRAM of FM77L2, bank = 64Kbytes")
set(FM77L4_EXTRAM_PAGES  "3" CACHE STRING "Set banks of EXTRAM of FM77L4, bank = 64Kbytes")
set(FM77AV40_EXTRAM_PAGES  "12" CACHE STRING "Set banks of EXTRAM of FM77AV40, bank = 64Kbytes")
set(FM77AV40SX_EXTRAM_PAGES  "12" CACHE STRING "Set banks of EXTRAM of FM77AV40SX, bank = 64Kbytes")
set(FM77AV40EX_EXTRAM_PAGES  "12" CACHE STRING "Set banks of EXTRAM of FM77AV40SX, bank = 64Kbytes")

set(FM7_DEBUG_FDC  OFF CACHE BOOL "With debug FDC")

include(detect_target_cpu)
# set entry

add_definitions(-D_CONFIGURE_WITH_CMAKE)

if(FM7_DEBUG_FDC)
  add_definitions(-D_FM7_FDC_DEBUG)
  add_definitions(-D_DEBUG_LOG)
endif()

if(BUILD_FM7)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fm7.qrc)
	ADD_VM(fm7 emufm7 _FM7)
endif()
if(BUILD_FM8)
	#set(VMFILES_LIB_FM7 ${VMFILES_LIB_FM7} ay_3_891x.cpp)
	#set(FLAG_USE_Z80 ON)
	#add_definitions(-DBUILD_Z80)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fm8.qrc)
	ADD_VM(fm7 emufm8 _FM8)
endif()
if(BUILD_FM77)
	#set(VMFILES_LIB_FM7 ${VMFILES_LIB_FM7} ay_3_891x.cpp)
	#set(FLAG_USE_Z80 ON)
	#add_definitions(-DBUILD_Z80)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fm77.qrc)
	ADD_VM(fm7 emufm77 _FM77)
	target_compile_definitions(emufm77
		PRIVATE -DFM77_EXRAM_BANKS=${FM77_EXTRAM_PAGES}
	)
endif()
if(BUILD_FM77L4)
	#set(VMFILES_LIB_FM7 ${VMFILES_LIB_FM7} ay_3_891x.cpp)
	#set(FLAG_USE_Z80 ON)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fm77.qrc)
	ADD_VM(fm7 emufm77l4 _FM77L4)
	target_compile_definitions(emufm77l4
		PRIVATE -DFM77_EXRAM_BANKS=${FM77L4_EXTRAM_PAGES}
	)
endif()
if(BUILD_FM77AV)
	#set(VMFILES_LIB_FM7 ${VMFILES_LIB_FM7} beep.cpp)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fm77av.qrc)
	ADD_VM(fm7 emufm77av _FM77AV)
endif()
if(BUILD_FM77AV20)
	#set(VMFILES_LIB_FM7 ${VMFILES_LIB_FM7} beep.cpp)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fm77av20.qrc)
	ADD_VM(fm7 emufm77av20 _FM77AV20)
endif()
if(BUILD_FM77AV20EX)
	#set(VMFILES_LIB_FM7 ${VMFILES_LIB_FM7} beep.cpp)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fm77av20ex.qrc)
	ADD_VM(fm7 emufm77av20ex _FM77AV20EX)
endif()
if(BUILD_FM77AV40)
	#set(VMFILES_LIB_FM7 ${VMFILES_LIB_FM7} beep.cpp)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fm77av40.qrc)
	ADD_VM(fm7 emufm77av40 _FM77AV40)
	target_compile_definitions(emufm77av40
		PRIVATE -DFM77_EXRAM_BANKS=${FM77AV40_EXTRAM_PAGES}
	)
endif()
if(BUILD_FM77AV40SX)
	#set(VMFILES_LIB_FM7 ${VMFILES_LIB_FM7} beep.cpp)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fm77av40sx.qrc)
	ADD_VM(fm7 emufm77av40sx _FM77AV40SX)
	target_compile_definitions(emufm77av40sx
		PRIVATE -DFM77_EXRAM_BANKS=${FM77AV40SX_EXTRAM_PAGES}
	)
endif()
if(BUILD_FM77AV40EX)
	#set(VMFILES_LIB_FM7 ${VMFILES_LIB_FM7} beep.cpp)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fm77av40ex.qrc)
	ADD_VM(fm7 emufm77av40ex _FM77AV40EX)
	target_compile_definitions(emufm77av40ex
		PRIVATE -DFM77_EXRAM_BANKS=${FM77AV40EX_EXTRAM_PAGES}
	)
endif()



                         


