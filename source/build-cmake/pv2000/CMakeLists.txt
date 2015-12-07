# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,CASIO PV-2000, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (pv2000)

set(LOCAL_LIBS 	   qt_osd
		   qt_debugger
		   qt_pv2000
		   qt_gui
		   vm_pv2000
		   vm_vm
		   common_common		   
                   )

set(VMFILES
		   z80.cpp
		   sn76489an.cpp
		   tms9918a.cpp
		   
		   io.cpp
		   memory.cpp
  
		   event.cpp
)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

include(detect_target_cpu)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_PV2000)
set(EXEC_TARGET emupv2000)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pv2000.qrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pv2000)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/machines/pv2000)

include(config_commonsource)

add_subdirectory(../../src/vm/pv2000 vm/pv2000)
add_subdirectory(../../src/qt/machines/pv2000 qt/pv2000)
add_subdirectory(../../src/qt/debugger qt/debugger)
add_subdirectory(../../src/qt/common qt/common)
