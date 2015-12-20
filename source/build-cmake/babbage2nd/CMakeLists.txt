# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,Babbage2nd, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emubabbage2nd)
set(EXEC_TARGET emubabbage2nd)
set(VM_NAME babbage2nd)
set(USE_FMGEN OFF)
set(VMFILES_BASE
  z80.cpp
  z80ctc.cpp
  z80pio.cpp
  
  io.cpp
  memory.cpp

  event.cpp
)


set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER OFF CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_BABBAGE2ND)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/babbage2nd.qrc)


include(config_commonsource)
