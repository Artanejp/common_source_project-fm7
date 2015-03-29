# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,FP1100, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emufp1100)


set(LOCAL_LIBS 	   vm_fp1100
		   vm_vm
		   common_common
#		   common_scaler-generic
                   qt_fp1100
		   qt_gui
                  )

set(VMFILES
		   z80.cpp
		   upd7801.cpp

#		   pcm1bit.cpp
		   beep.cpp
		   upd765a.cpp
		   hd46505.cpp
		   
		   ym2203.cpp
		   
		   datarec.cpp
		   disk.cpp
		   event.cpp
		   io.cpp
		   memory.cpp
)


set(BUILD_SHARED_LIBS OFF)


set(USE_CMT_SOUND ON CACHE BOOL "Sound with Data Recorder.")
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL if enabled.")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

#set(WITH_DEBUGGER ON CACHE BOOL "Build witn FP1100 Debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)

  set(EXEC_TARGET emufp1100)
  add_definitions(-D_FP1100)

if(USE_CMT_SOUND)
  add_definitions(-DDATAREC_SOUND)
endif()


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fp1100)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/fp1100)

include(config_commonsource)

if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/fp1100 vm/fp1100)
add_subdirectory(../../src/qt/fp1100 qt/fp1100)

                           
add_subdirectory(../../src/qt/common qt/common)
