# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,Mitsubishi Multi 8, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emumulti8)

set(EXEC_TARGET emumulti8)

set(LOCAL_LIBS 	   vm_multi8
		   vm_vm
		   vm_fmgen
		   common_common
		   qt_multi8
		   qt_debugger
		   qt_gui
                   )

set(VMFILES_BASE
                   z80.cpp
		   
		   hd46505.cpp
		   i8251.cpp
		   i8253.cpp
		   i8255.cpp
		   i8259.cpp
		   upd765a.cpp
		   ym2203.cpp
		   
		   io.cpp
		   datarec.cpp
		   disk.cpp
		   
		   event.cpp
)


set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
#set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_MULTI8)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/multi8.qrc)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/multi8)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/multi8)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/debugger)

include(config_commonsource)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)

add_subdirectory(../../src/vm/multi8 vm/multi8)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/multi8 qt/multi8)
add_subdirectory(../../src/qt/debugger qt/debugger)
     
add_subdirectory(../../src/qt/common qt/common)
