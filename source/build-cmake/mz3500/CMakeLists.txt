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

set(LOCAL_LIBS 	   vm_mz3500
		   vm_vm
#		   vm_fmgen
		   common_common
		   qt_mz3500
		   qt_gui
                   )

set(VMFILES
		   z80.cpp
		   
		   i8251.cpp
		   i8253.cpp
		   i8255.cpp
		   
		   ls244.cpp
		   upd1990a.cpp
		   upd7220.cpp
		   upd765a.cpp
		   
		   pcm1bit.cpp
		   
		   disk.cpp
		   event.cpp
		   io.cpp
)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_MZ3500)
set(EXEC_TARGET emumz3500)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz3500.qrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/mz3500)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/mz3500)

include(config_commonsource)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)

add_subdirectory(../../src/vm/mz3500 vm/mz3500)
add_subdirectory(../../src/qt/mz3500 qt/mz3500)
#add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/common qt/common)
