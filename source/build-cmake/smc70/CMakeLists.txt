# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,SONY SMC-70, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emusmc70)

set(EXEC_TARGET emusmc70)

set(LOCAL_LIBS 	   qt_osd
		   qt_smc777
		   qt_debugger
		   qt_gui
		   vm_smc777
		   vm_vm
		   common_common		   
                   )

set(VMFILES_BASE
                   z80.cpp
		   
		   pcm1bit.cpp
		   hd46505.cpp
		   mb8877.cpp
		   sn76489an.cpp
		   
		   datarec.cpp
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
add_definitions(-D_SMC777)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/smc70.qrc)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/smc777)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/machines/smc777)

include(config_commonsource)

add_subdirectory(../../src/vm/smc777 vm/smc777)
add_subdirectory(../../src/qt/machines/smc777 qt/smc777)
     
add_subdirectory(../../src/qt/common qt/common)
add_subdirectory(../../src/qt/debugger qt/debugger)
