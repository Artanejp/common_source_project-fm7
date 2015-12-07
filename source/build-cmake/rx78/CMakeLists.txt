# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,BANDAI RX-78, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (rx78)

set(LOCAL_LIBS 	   qt_osd
		   qt_debugger
		   qt_rx78
		   qt_gui
		   vm_rx78
		   vm_vm
		   common_common		   
                   )

set(VMFILES
		   z80.cpp
		   sn76489an.cpp
		   datarec.cpp
		   
		   io.cpp
		   event.cpp
)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

include(detect_target_cpu)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_RX78)
set(EXEC_TARGET emurx78)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/rx78.qrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/rx78)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/machines/rx78)

include(config_commonsource)

add_subdirectory(../../src/vm/rx78 vm/rx78)
add_subdirectory(../../src/qt/machines/rx78 qt/rx78)
add_subdirectory(../../src/qt/debugger qt/debugger)
add_subdirectory(../../src/qt/common qt/common)
