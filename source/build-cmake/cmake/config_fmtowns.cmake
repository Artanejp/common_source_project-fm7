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
  
  scsi_host.cpp

  msm58321.cpp
  
)
set(VMFILES_LIB
   i386.cpp
   noise.cpp
   pcm1bit.cpp
   i8251.cpp
   i8253.cpp
   i8259.cpp
   io.cpp
   upd71071.cpp
   mb8877.cpp
   
   scsi_dev.cpp
   scsi_hdd.cpp
   scsi_cdrom.cpp
   
   disk.cpp
   prnfile.cpp
   harddisk.cpp
)

set(FLAG_USE_I386 ON)

set(BUILD_FMTOWNS_1 OFF CACHE BOOL "Build for FM-Towns Model 1")
set(BUILD_FMTOWNS_2 OFF CACHE BOOL "Build for FM-Towns Model 2")
set(BUILD_FMTOWNS_2F OFF CACHE BOOL "Build for FM-Towns 2F")
set(BUILD_FMTOWNS_2H OFF CACHE BOOL "Build for FM-Towns 2H")
set(BUILD_FMTOWNS_20F OFF CACHE BOOL "Build for FM-Towns 20F")
set(BUILD_FMTOWNS_20H OFF CACHE BOOL "Build for FM-Towns 20H")
set(BUILD_FMTOWNS2_UX20 OFF CACHE BOOL "Build for FM-Towns2 UX20")
set(BUILD_FMTOWNS2_UX40 OFF CACHE BOOL "Build for FM-Towns2 UX40")
set(BUILD_FMTOWNS2_CX20 OFF CACHE BOOL "Build for FM-Towns2 CX20")
set(BUILD_FMTOWNS2_CX40 OFF CACHE BOOL "Build for FM-Towns2 CX40")
set(BUILD_FMTOWNS2_CX100 OFF CACHE BOOL "Build for FM-Towns2 CX100")
set(BUILD_FMTOWNS2_UG10 OFF CACHE BOOL "Build for FM-Towns2 UG10")
set(BUILD_FMTOWNS2_UG20 OFF CACHE BOOL "Build for FM-Towns2 UG20")
set(BUILD_FMTOWNS2_UG40 OFF CACHE BOOL "Build for FM-Towns2 UG40")
set(BUILD_FMTOWNS2_UG80 OFF CACHE BOOL "Build for FM-Towns2 UG80")
set(BUILD_FMTOWNS2_HG20 OFF CACHE BOOL "Build for FM-Towns2 HG20")
set(BUILD_FMTOWNS2_HG40 OFF CACHE BOOL "Build for FM-Towns2 HG40")
set(BUILD_FMTOWNS2_HG100 OFF CACHE BOOL "Build for FM-Towns2 HG100")
set(BUILD_FMTOWNS2_HR20 OFF CACHE BOOL "Build for FM-Towns2 HR20")
set(BUILD_FMTOWNS2_HR100 OFF CACHE BOOL "Build for FM-Towns2 HR100")
set(BUILD_FMTOWNS2_HR200 OFF CACHE BOOL "Build for FM-Towns2 HR200")

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

if(BUILD_FMTOWNS_1)
  set(EXEC_TARGET emufmtowns)
  add_definitions(-D_FMTOWNS_1)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS_2)
  set(EXEC_TARGET emufmtowns_2)
  add_definitions(-D_FMTOWNS_2)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS_2F)
  set(EXEC_TARGET emufmtowns2F)
  add_definitions(-D_FMTOWNS_2F)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS_2H)
  set(EXEC_TARGET emufmtowns2H)
  add_definitions(-D_FMTOWNS_2H)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS_20F)
  set(EXEC_TARGET emufmtowns20F)
  add_definitions(-D_FMTOWNS_20F)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS_20H)
  set(EXEC_TARGET emufmtowns20H)
  add_definitions(-D_FMTOWNS_20H)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS2_UX20)
  set(EXEC_TARGET emufmtowns2UX20)
  add_definitions(-D_FMTOWNS2_UX20)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS2_UX40)
  set(EXEC_TARGET emufmtowns2UX40)
  add_definitions(-D_FMTOWNS2_UX40)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS2_CX20)
  set(EXEC_TARGET emufmtowns2CX20)
  add_definitions(-D_FMTOWNS2_CX20)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS2_CX40)
  set(EXEC_TARGET emufmtowns2CX40)
  add_definitions(-D_FMTOWNS2_CX40)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS2_CX100)
  set(EXEC_TARGET emufmtowns2CX100)
  add_definitions(-D_FMTOWNS2_CX100)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS2_UG10)
  set(EXEC_TARGET emufmtowns2UG10)
  add_definitions(-D_FMTOWNS2_UG10)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS2_UG20)
  set(EXEC_TARGET emufmtowns2UG20)
  add_definitions(-D_FMTOWNS2_UG20)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS2_UG40)
  set(EXEC_TARGET emufmtowns2UG40)
  add_definitions(-D_FMTOWNS2_UG40)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS2_UG80)
  set(EXEC_TARGET emufmtowns2UG80)
  add_definitions(-D_FMTOWNS2_UG80)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS2_HG20)
  set(EXEC_TARGET emufmtowns2HG20)
  add_definitions(-D_FMTOWNS2_HG20)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS2_HG40)
  set(EXEC_TARGET emufmtowns2HG40)
  add_definitions(-D_FMTOWNS2_HG40)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS2_HG100)
  set(EXEC_TARGET emufmtowns2HG100)
  add_definitions(-D_FMTOWNS2_HG100)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS2_HR20)
  set(EXEC_TARGET emufmtowns2HR20)
  add_definitions(-D_FMTOWNS2_HR20)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS2_HR100)
  set(EXEC_TARGET emufmtowns2HR100)
  add_definitions(-D_FMTOWNS2_HR100)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
elseif(BUILD_FMTOWNS2_HR200)
  set(EXEC_TARGET emufmtowns2HR200)
  add_definitions(-D_FMTOWNS2_HR200)
  ## ToDo
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmtowns.qrc)
endif()

include(config_commonsource)
   

