# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,SORD m5, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emum5)

set(EXEC_TARGET emum5)

set(LOCAL_LIBS 	   vm_m5
		   vm_vm
		   common_common
		   qt_debugger
		   qt_m5
		   qt_gui
                   )

set(VMFILES_BASE
  z80.cpp
  sn76489an.cpp
  tms9918a.cpp
  z80ctc.cpp
  memory.cpp
  
  datarec.cpp
  io.cpp
  event.cpp
)


set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
#set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_M5)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/m5.qrc)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/m5)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/m5)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/debugger)

include(config_commonsource)

add_subdirectory(../../src/vm/m5 vm/m5)
add_subdirectory(../../src/qt/m5 qt/m5)
add_subdirectory(../../src/qt/debugger qt/debugger)
     
add_subdirectory(../../src/qt/common qt/common)
