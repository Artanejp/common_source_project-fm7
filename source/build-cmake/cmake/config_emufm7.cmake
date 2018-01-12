# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

message("")
message("** Start of configure CommonSourceProject,FM-8/7/77/AV, Qt **")
message("")

set(VM_NAME fm7)
set(USE_FMGEN ON)
set(WITH_DEBUGGER ON)
set(WITH_MOUSE ON)
set(WITH_JOYSTICK ON)

set(VMFILES
		   event.cpp
		   io.cpp
)

set(VMFILES_LIB
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

set(FLAG_USE_MC6809 ON)

if(NOT BUILD_FM7)
 set(BUILD_FM7 OFF CACHE BOOL "Build for FM7")
endif()

if(NOT BUILD_FM8)
 set(BUILD_FM8 OFF CACHE BOOL "Build for FM8")
endif()

if(NOT BUILD_FM77)
 set(BUILD_FM77 OFF CACHE BOOL "Build for FM77")
endif()

if(NOT BUILD_FM77L2)
 set(BUILD_FM77L2 OFF CACHE BOOL "Build for FM77L2")
endif()

if(NOT BUILD_FM77L4)
 set(BUILD_FM77L4 OFF CACHE BOOL "Build for FM77L4")
endif()

if(NOT BUILD_FM77AV)
 set(BUILD_FM77AV OFF CACHE BOOL "Build for FM77AV")
endif()

if(NOT BUILD_FM77AV20)
 set(BUILD_FM77AV20 OFF CACHE BOOL "Build for FM77AV20")
endif()

if(NOT BUILD_FM77AV40)
 set(BUILD_FM77AV40 OFF CACHE BOOL "Build for FM77AV40")
endif()

if(NOT BUILD_FM77AV40SX)
 set(BUILD_FM77AV40SX OFF CACHE BOOL "Build for FM77AV40SX")
endif()

if(NOT BUILD_FM77AV40EX)
 set(BUILD_FM77AV40EX OFF CACHE BOOL "Build for FM77AV40EX")
endif()

set(FM77_EXTRAM_PAGES  "12" CACHE STRING "Set banks of EXTRAM of FM77/FM77AV40, bank = 64Kbytes")


set(BUILD_SHARED_LIBS OFF)
set(FM77_EXTRAM_PAGES  "12" CACHE STRING "Set banks of EXTRAM of FM77/FM77AV40, bank = 64Kbytes")
set(FM7_DEBUG_FDC  OFF CACHE BOOL "With debug FDC")
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)

if(FM7_DEBUG_FDC)
  add_definitions(-D_FM7_FDC_DEBUG)
  add_definitions(-D_DEBUG_LOG)
endif()

if(BUILD_FM7)
  set(EXEC_TARGET emufm7)
  add_definitions(-D_FM7)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fm7.qrc)
  set(VMFILES_LIB ${VMFILES_LIB} ay_3_891x.cpp)
  set(FLAG_USE_Z80 ON)
  add_definitions(-DBUILD_Z80)
 
elseif(BUILD_FM8)
  set(EXEC_TARGET emufm8)
  add_definitions(-D_FM8)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fm8.qrc)
  set(VMFILES_LIB ${VMFILES_LIB} ay_3_891x.cpp)
  set(FLAG_USE_Z80 ON)
  add_definitions(-DBUILD_Z80)
  
elseif(BUILD_FM77)
  set(EXEC_TARGET emufm77)
  add_definitions(-D_FM77)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fm77.qrc)
  set(VMFILES_LIB ${VMFILES_LIB} ay_3_891x.cpp)
  set(FLAG_USE_Z80 ON)
  add_definitions(-DBUILD_Z80)
  
elseif(BUILD_FM77L2)
  set(EXEC_TARGET emufm77l2)
  add_definitions(-D_FM77L2)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fm77l2.qrc)
  set(VMFILES_LIB ${VMFILES_LIB} ay_3_891x.cpp)
  set(FLAG_USE_Z80 ON)
  add_definitions(-DBUILD_Z80)
  
elseif(BUILD_FM77L4)
  set(EXEC_TARGET emufm77l4)
  add_definitions(-D_FM77L4)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/source/src/qt/fm7/fm77.qrc)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fm77l4.qrc)
  set(VMFILES_LIB ${VMFILES_LIB} ay_3_891x.cpp)
  set(FLAG_USE_Z80 ON)
  add_definitions(-DBUILD_Z80)

elseif(BUILD_FM77AV)
  set(EXEC_TARGET emufm77av)
  add_definitions(-D_FM77AV)
  set(FM77AV_VARIANTS ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fm77av.qrc)
  set(VMFILES_LIB ${VMFILES_LIB} beep.cpp)

elseif(BUILD_FM77AV20)
  set(EXEC_TARGET emufm77av20)
  add_definitions(-D_FM77AV20)
  set(FM77AV_VARIANTS ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fm77av20.qrc)
  set(VMFILES_LIB ${VMFILES_LIB} beep.cpp)
  
elseif(BUILD_FM77AV20EX)
  set(EXEC_TARGET emufm77av20ex)
  add_definitions(-D_FM77AV20EX)
  set(FM77AV_VARIANTS ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fm77av20ex.qrc)
  set(VMFILES_LIB ${VMFILES_LIB} beep.cpp)
  
elseif(BUILD_FM77AV40)
  set(EXEC_TARGET emufm77av40)
  add_definitions(-D_FM77AV40)
  set(FM77AV_VARIANTS ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fm77av40.qrc)
  set(VMFILES_LIB ${VMFILES_LIB} beep.cpp)

elseif(BUILD_FM77AV40SX)
  set(EXEC_TARGET emufm77av40sx)
  add_definitions(-D_FM77AV40SX)
  set(FM77AV_VARIANTS ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fm77av40sx.qrc)
  set(VMFILES_LIB ${VMFILES_LIB} beep.cpp)

elseif(BUILD_FM77AV40EX)
  set(EXEC_TARGET emufm77av40ex)
  add_definitions(-D_FM77AV40EX)
  set(FM77AV_VARIANTS ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fm77av40ex.qrc)
  set(VMFILES_LIB ${VMFILES_LIB} beep.cpp)
  
endif()

add_definitions(-DFM77_EXRAM_BANKS=${FM77_EXTRAM_PAGES})


                         


