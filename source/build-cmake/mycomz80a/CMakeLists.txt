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

set(LOCAL_LIBS 	   vm_mycomz80a
		   vm_vm
		   common_common
		   qt_mycomz80a
		   qt_gui
                   )

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
set(XM7_VERSION 3)
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_MYCOMZ80A)
set(EXEC_TARGET emumycomz80a)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mycomz80a.qrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/mycomz80a)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/mycomz80a)

include(config_commonsource)

add_subdirectory(../../src/vm/mycomz80a vm/mycomz80a)
add_subdirectory(../../src/qt/mycomz80a qt/mycomz80a)
add_subdirectory(../../src/qt/common qt/common)
