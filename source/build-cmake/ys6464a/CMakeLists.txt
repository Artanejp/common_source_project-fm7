# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,SHINKO SANGYOU  YS6464A, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emuys6464a)

set(EXEC_TARGET emuys6464a)

set(LOCAL_LIBS 	   qt_osd
		   qt_ys6464a
		   qt_gui
		   vm_ys6464a
		   vm_vm
		   common_common		   
                   )

set(VMFILES_BASE
  z80.cpp
  i8255.cpp
 
  io.cpp
  memory.cpp

  event.cpp
)


set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER OFF CACHE BOOL "Build with debugger.")


include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})

add_definitions(-D_YS6464A)
add_definitions(-DWITHOUT_SOUND)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/ys6464a.qrc)


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/ys6464a)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/machines/ys6464a)

include(config_commonsource)

add_subdirectory(../../src/vm/ys6464a vm/ys6464a)
add_subdirectory(../../src/qt/machines/ys6464a qt/ys6464a)
     
add_subdirectory(../../src/qt/common qt/common)
