# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,IBM JX, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emujx)

set(EXEC_TARGET emujx)
set(VM_NAME jx)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE ON)

set(VMFILES_BASE
      i286.cpp
      io.cpp
#      memory.cpp
      event.cpp
)
set(VMFILES_LIB
      disk.cpp
      i8251.cpp
      i8253.cpp
      i8255.cpp
      i8259.cpp
	  
      hd46505.cpp
      not.cpp
	  noise.cpp
      pcm1bit.cpp
      upd765a.cpp
      sn76489an.cpp
)
set(FLAG_USE_I86 OFF)

set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger")
include(detect_target_cpu)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_JX)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/jx.qrc)

include(config_commonsource)
