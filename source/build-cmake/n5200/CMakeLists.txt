# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,NEC N5200, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emun5200)

set(EXEC_TARGET emun5200)

set(LOCAL_LIBS 	   vm_n5200
		   vm_vm
		   common_common
		   qt_n5200
		   qt_gui
                   )

set(VMFILES_BASE
                   i386.cpp
		   
		   beep.cpp
		   i8237.cpp
		   i8251.cpp
		   i8253.cpp
		   i8255.cpp
		   i8259.cpp
		   
		   upd1990a.cpp
		   upd7220.cpp
		   upd765a.cpp
		   
		   io.cpp
		   disk.cpp
		   event.cpp
)


set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_N5200)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/n5200.qrc)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/n5200)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/n5200)

include(config_commonsource)

add_subdirectory(../../src/vm/n5200 vm/n5200)
add_subdirectory(../../src/qt/n5200 qt/n5200)
     
add_subdirectory(../../src/qt/common qt/common)
