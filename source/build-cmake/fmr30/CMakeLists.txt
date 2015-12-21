# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,FMR 30, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emufmr30)
set(VM_NAME fmr30)
set(USE_FMGEN OFF)
set(EXEC_TARGET emufmr30)
set(VMFILES
		   i286.cpp
		   pcm1bit.cpp
		   i8237.cpp
		   i8251.cpp
		   i8253.cpp
		   i8259.cpp
		   fmr50/bios.cpp
		   
		   sn76489an.cpp
		   mb8877.cpp

		   io.cpp

		   disk.cpp
		   event.cpp
)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)
add_definitions(-D_FMR30)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmr30.qrc)

include(config_commonsource)
