# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,FP1100, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emufp1100)
set(VM_NAME fp1100)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE OFF)

set(FLAG_USE_Z80 ON)
set(VMFILES
		  event.cpp
		  io.cpp
		  memory.cpp
)
set(VMFILES_LIB
		   datarec.cpp
		   upd7801.cpp
		   upd765a.cpp
#		   pcm1bit.cpp
		   ym2203.cpp
		   hd46505.cpp
		   beep.cpp
		   disk.cpp
)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with FP1100 debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)

  set(EXEC_TARGET emufp1100)
  add_definitions(-D_FP1100)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fp1100.qrc)


include(config_commonsource)
