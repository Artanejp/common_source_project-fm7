# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,SEIKO MAP1010, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (map1010)

set(LOCAL_LIBS 	   vm_phc25
		   vm_vm
		   vm_fmgen
		   common_common
		   qt_debugger
		   qt_phc25
		   qt_gui
                   )

set(VMFILES
		   z80.cpp
		   not.cpp
		   mc6847.cpp
		   ym2203.cpp
		   
		   datarec.cpp
		   io.cpp
		   
		   event.cpp
)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(BUILD_PHC25 OFF CACHE BOOL "Build ePHC25")
set(BUILD_MAP1010 ON CACHE BOOL "Build eMAP1010")

if(BUILD_PHC25)
  add_definitions(-D_PHC25)
  set(EXEC_TARGET emuphc25)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/phc25.qrc)
elseif(BUILD_MAP1010)
  add_definitions(-D_MAP1010)
  set(EXEC_TARGET emumap1010)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/map1010.qrc)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/phc25)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/phc25)

include(config_commonsource)

add_subdirectory(../../src/vm/phc25 vm/phc25)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/phc25 qt/phc25)
add_subdirectory(../../src/qt/common qt/common)
add_subdirectory(../../src/qt/debugger qt/debugger)
