# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,MYCOM Z80A, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (mz3500)
set(VM_NAME mz3500)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE ON)

set(VMFILES
#	   z80.cpp
	   
	   upd7220.cpp
   
	   mz1p17.cpp
	   event.cpp
	   io.cpp
)
set(VMFILES_LIB
	   i8251.cpp
	   i8253.cpp
	   i8255.cpp
		   
	   ls244.cpp
	   not.cpp
	   noise.cpp
	   pcm1bit.cpp
	   upd1990a.cpp
	   upd765a.cpp
	   upd7220_base.cpp
	   
	   prnfile.cpp
	   
	   disk.cpp
)	   
set(FLAG_USE_Z80 ON)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_MZ3500)
set(EXEC_TARGET emumz3500)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz3500.qrc)

include(config_commonsource)
