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

project (mz2800)
set(VM_NAME mz2800)
set(USE_FMGEN ON)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE ON) 

set(VMFILES
		   i8253.cpp
		   i8255.cpp
		   i8259.cpp
		   rp5c01.cpp

		   z80pio.cpp
		   z80sio.cpp
		   mz1p17.cpp
		   prnfile.cpp
		   
		   event.cpp
		   io.cpp
)
set(VMFILES_LIB
#		   i286.cpp
		upd71071.cpp
		mb8877.cpp
		not.cpp
		pcm1bit.cpp
		ym2203.cpp
		disk.cpp
)
set(FLAG_USE_I286 ON)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_MZ2800)
set(EXEC_TARGET emumz2800)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz2800.qrc)

include(config_commonsource)
