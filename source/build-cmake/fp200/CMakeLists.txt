# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,FP200, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emufp200)


set(LOCAL_LIBS 	   vm_fp200
		   vm_vm
		   common_common
                   qt_fp200
		   qt_gui
		   qt_debugger
                  )

set(VMFILES
		   i8080.cpp
#
		   rp5c01.cpp
		   
		   datarec.cpp
		   memory.cpp
		   event.cpp
)


set(BUILD_SHARED_LIBS OFF)


set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)

set(EXEC_TARGET emufp200)
add_definitions(-D_FP200)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fp200.qrc)


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fp200)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/fp200)

include(config_commonsource)

if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/fp200 vm/fp200)
add_subdirectory(../../src/qt/fp200 qt/fp200)

add_subdirectory(../../src/qt/common qt/common)
