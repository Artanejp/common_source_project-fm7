# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,Sharp MZ5500/6500/6550, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (mz5500)

set(LOCAL_LIBS 	   vm_mz5500
		   vm_vm
		   vm_fmgen
		   common_common
		   qt_mz5500
		   qt_debugger
		   qt_gui
                   )

set(VMFILES
		   i286.cpp
		   
		   i8237.cpp
		   i8255.cpp
		   i8259.cpp
		   
		   ls393.cpp
		   rp5c01.cpp
		   upd7220.cpp
		   upd765a.cpp
		   ym2203.cpp
		   z80ctc.cpp
		   z80sio.cpp
		   
		   disk.cpp
		   event.cpp
		   io.cpp
)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(BUILD_MZ5500 OFF CACHE BOOL "Build emumz5500")
set(BUILD_MZ6500 ON  CACHE BOOL "Build emumz6500")
set(BUILD_MZ6550 OFF CACHE BOOL "Build emumz6550")

#set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

if(BUILD_MZ5500)
  add_definitions(-D_MZ5500)
  set(EXEC_TARGET emumz5500)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz5500.qrc)
elseif(BUILD_MZ6500)
  add_definitions(-D_MZ6500)
  set(EXEC_TARGET emumz6500)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz6500.qrc)
else()
  add_definitions(-D_MZ6550)
  set(EXEC_TARGET emumz6550)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz6550.qrc)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/mz5500)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/mz5500)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/debugger)

include(config_commonsource)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)

add_subdirectory(../../src/vm/mz5500 vm/mz5500)
add_subdirectory(../../src/qt/mz5500 qt/mz5500)
add_subdirectory(../../src/qt/debugger qt/debugger)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/common qt/common)
