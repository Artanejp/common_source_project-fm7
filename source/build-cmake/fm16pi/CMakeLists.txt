# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,FM16π, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emufm16pi)


set(LOCAL_LIBS 	   vm_fm16pi
		   vm_vm
		   common_common
                   qt_fm16pi
		   qt_gui
                  )

set(VMFILES
		   i286.cpp
#
		   pcm1bit.cpp
		   i8251.cpp
		   i8253.cpp
		   i8255.cpp
		   i8259.cpp
		   msm58321.cpp
		   not.cpp
		   mb8877.cpp
		   
		   ym2203.cpp
		   
#		   datarec.cpp
		   disk.cpp
		   event.cpp
		   io.cpp
		   memory.cpp
)


set(BUILD_SHARED_LIBS OFF)

#set(FM77_EXTRAM_PAGES  "12" CACHE STRING "Set banks of EXTRAM of FM77/FM77AV40, bank = 64Kbytes")

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL if enabled.")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)

set(EXEC_TARGET emufm16pi)
add_definitions(-D_FM16PI)
  



#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm16pi)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/fm7)

include(config_commonsource)

if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/fm16pi vm/fm16pi)
add_subdirectory(../../src/qt/fm16pi qt/fm16pi)


                           
add_subdirectory(../../src/qt/common qt/common)
