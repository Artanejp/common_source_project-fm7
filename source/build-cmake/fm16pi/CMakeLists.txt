# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,FM16π, Qt **")
message("")



project (emufm16pi)
set(CMAKE_MODULE_PATH "${emufm16pi_SOURCE_DIR}/../cmake")


set(EXEC_TARGET emufm16pi)
set(VM_NAME fm16pi)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE ON)

set(VMFILES
		   i286.cpp
		   msm58321.cpp
		   event.cpp
		   io.cpp
		   memory.cpp
)

set(VMFILES_LIB
#		   datarec.cpp
		   disk.cpp
		   i8251.cpp
		   i8253.cpp
		   i8255.cpp
		   i8259.cpp
		   mb8877.cpp
		   msm58321_base.cpp
		   noise.cpp
		   not.cpp
		   pcm1bit.cpp
		   ym2203.cpp
)

#set(FLAG_USE_I86 ON)
set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)
add_definitions(-D_FM16PI)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fm16pi.qrc)

include(config_commonsource)
