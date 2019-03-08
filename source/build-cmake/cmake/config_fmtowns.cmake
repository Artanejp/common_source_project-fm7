# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

set(VM_NAME fmtowns)
set(USE_FMGEN ON)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE ON)
set(VMFILES
  event.cpp
  io.cpp
  
  scsi_host.cpp
  scsi_dev.cpp
  scsi_hdd.cpp
  scsi_cdrom.cpp
)
set(VMFILES_LIB
   noise.cpp
   pcm1bit.cpp
   i8251.cpp
   i8253.cpp
   i8259.cpp
   msm58321.cpp
   upd71071.cpp
   
   mb8877.cpp
   ym2151.cpp
   
   disk.cpp
   prnfile.cpp
   harddisk.cpp
)

set(FLAG_USE_I386 ON)

set(BUILD_FMTOWNS_2 OFF CACHE BOOL "Build for FM-Towns Model 2")
set(BUILD_FMTOWNS_2H OFF CACHE BOOL "Build for FM-Towns 2H")
set(BUILD_FMTOWNS_20H OFF CACHE BOOL "Build for FM-Towns 20H")
set(BUILD_FMTOWNS2_UX40 OFF CACHE BOOL "Build for FM-Towns2 UX40")
set(BUILD_FMTOWNS2_CX100 OFF CACHE BOOL "Build for FM-Towns2 CX100")

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

if(BUILD_FMTOWNS_2)
  set(EXEC_TARGET emufmtowns_2)
  add_definitions(-D_FMTOWNS_2)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/x1.qrc)
elseif(BUILD_FMTOWNS_2H)
  set(EXEC_TARGET emufmtowns2H)
  add_definitions(-D_FMTOWNS_2H)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/x1.qrc)
elseif(BUILD_FMTOWNS_20H)
  set(EXEC_TARGET emufmtowns40H)
  add_definitions(-D_FMTOWNS_40H)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/x1.qrc)
elseif(BUILD_FMTOWNS2_UX40)
  set(EXEC_TARGET emufmtownsUX40)
  add_definitions(-D_FMTOWNS2_UX40)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/x1.qrc)
elseif(BUILD_FMTOWNS2_CX100)
  set(EXEC_TARGET emufmtownsCX100)
  add_definitions(-D_FMTOWNS2_CX100)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/x1.qrc)
endif()

include(config_commonsource)
   

