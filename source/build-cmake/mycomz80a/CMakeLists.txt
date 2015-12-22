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

project (mycomz80a)
set(VM_NAME mycomz80a)
set(USE_FMGEN OFF)
set(VMFILES
	z80.cpp
	hd46505.cpp
	
	i8255.cpp
	msm58321.cpp
	sn76489an.cpp
	
	datarec.cpp
	
	event.cpp
	io.cpp
)

set(BUILD_MYCOMZ80A ON CACHE BOOL "Build EMU-MYCOMZ80A")
set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_MYCOMZ80A)
set(EXEC_TARGET emumycomz80a)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mycomz80a.qrc)

include(config_commonsource)
