# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,NEC PC-100, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (pc100)
set(VM_NAME pc100)
set(USE_FMGEN ON)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE ON)

set(VMFILES
#		   i286.cpp
		   msm58321.cpp
		   
		   memory.cpp
		   event.cpp
		   io.cpp
)
set(VMFILES_LIB
		   beep.cpp
		   and.cpp
		   i8251.cpp
		   i8255.cpp
		   i8259.cpp
		   msm58321_base.cpp
		   noise.cpp
		   pcm1bit.cpp
		   upd765a.cpp
		   disk.cpp
)
set(FLAG_USE_I86 ON)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger")
include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_PC100)
set(EXEC_TARGET emupc100)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc100.qrc)

include(config_commonsource)
