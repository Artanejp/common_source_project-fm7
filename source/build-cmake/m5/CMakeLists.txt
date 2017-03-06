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
set(VM_NAME m5)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE ON)

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

set(VMFILES_LIB
)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_M5)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/m5.qrc)

include(config_commonsource)
