# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,EPSON HC 80, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emujx)

set(EXEC_TARGET emujx)

set(LOCAL_LIBS 	   qt_osd
		   qt_jx
		   qt_gui
		   vm_jx
		   vm_vm
		   common_common
                   )

set(VMFILES_BASE

  i86.cpp
  
  hd46505.cpp
  i8251.cpp
  i8253.cpp
  i8255.cpp
  i8259.cpp
  io.cpp
  memory.cpp
  
  pcm1bit.cpp
  sn76489an.cpp
  not.cpp
  upd765a.cpp
  disk.cpp
  
  event.cpp
)

include(detect_target_cpu)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_JX)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/jx.qrc)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/jx)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/machines/jx)

include(config_commonsource)

add_subdirectory(../../src/vm/jx vm/jx)
add_subdirectory(../../src/qt/machines/jx qt/jx)
     
add_subdirectory(../../src/qt/common qt/common)
